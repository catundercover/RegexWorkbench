// Implements structural and Spot-backed omega regular-expression rewriting.
#include "automata/rewrite/OmegaRegexRewriter.hpp"

#include "automata/conversion/OmegaRegexAlphabet.hpp"
#include "automata/conversion/OmegaRegexToSpot.hpp"
#include "automata/conversion/SpotToOmegaRegex.hpp"
#include "regex/formatting/OmegaRegexFormatter.hpp"
#include "regex/inspection/OmegaExpressionInspection.hpp"
#include "regex/parsing/RegexParser.hpp"
#include "regex/simplification/OmegaRegexSimplifier.hpp"

#include <spot/twaalgos/postproc.hh>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>
#include <algorithm>

namespace automata::rewrite::omega
{
    namespace
    {
        struct Rewritten
        {
            regex::omega::Expression expression;
            std::size_t nodes;
        };

        [[noreturn]] void throw_budget_error()
        {
            throw std::runtime_error(
                "Rewriting would exceed the limit of " +
                std::to_string(automata::rewrite::MaxOutputNodes) +
                " expression nodes. Use a smaller expression or keep the abbreviation."
            );
        }

        Rewritten checked(regex::omega::Expression expression)
        {
            const std::size_t nodes = regex::inspection::omega::node_count(expression);
            if (nodes > automata::rewrite::MaxOutputNodes)
            {
                throw_budget_error();
            }
            return {std::move(expression), nodes};
        }

        void add_nodes(std::size_t& total, const std::size_t additional)
        {
            if (additional > automata::rewrite::MaxOutputNodes ||
                total > automata::rewrite::MaxOutputNodes - additional)
            {
                throw_budget_error();
            }

            total += additional;
        }

        template <typename Node>
        const Node& require_node(const std::shared_ptr<const Node>& node)
        {
            if (!node)
            {
                throw std::invalid_argument("Invalid null node during omega regex rewrite");
            }
            return *node;
        }

        regex::Expression alphabet_expression(const Alphabet& alphabet)
        {
            if (alphabet.empty())
            {
                throw std::invalid_argument("Cannot expand Sigma over an empty alphabet");
            }

            std::vector<Symbol> symbols(alphabet.begin(), alphabet.end());
            std::ranges::sort(symbols);

            std::vector<regex::Expression> alternatives;
            alternatives.reserve(symbols.size());
            for (const Symbol symbol : symbols)
            {
                alternatives.push_back(regex::make_terminal(symbol));
            }

            return regex::make_alternation(std::move(alternatives));
        }

        regex::Expression rewrite_finite_operand(
            const regex::Expression& expression, const Options& options, const Alphabet& alphabet
        )
        {
            automata::rewrite::Options finite_options;
            finite_options.remove_power = options.remove_power;
            finite_options.remove_plus = options.remove_plus;
            finite_options.remove_any_symbol = options.remove_any_symbol;
            finite_options.remove_complement = options.remove_complement;
            finite_options.remove_intersection = options.remove_intersection;

            const std::string rewritten =
                automata::rewrite::apply(expression, finite_options, alphabet);

            const regex::parsing::ParseResult parsed = regex::parsing::parse(rewritten);
            if (!parsed.expression || parsed.error)
            {
                throw std::runtime_error("Internal error: finite rewrite produced invalid regex");
            }

            return *parsed.expression;
        }

        Rewritten enforce_removed_abbreviations(
            const regex::omega::Expression& expression,
            const Options& options,
            const Alphabet& alphabet
        )
        {
            if (std::holds_alternative<regex::omega::EmptySet>(expression))
            {
                return {expression, 1};
            }

            if (std::holds_alternative<regex::omega::UniversalSet>(expression))
            {
                if (!options.remove_any_symbol)
                {
                    return {expression, 1};
                }

                return checked(regex::omega::make_omega_power(alphabet_expression(alphabet)));
            }

            if (const auto* power =
                    std::get_if<std::shared_ptr<const regex::omega::OmegaPower>>(&expression))
            {
                regex::Expression inner =
                    rewrite_finite_operand(require_node(*power).expression, options, alphabet);

                return checked(regex::omega::make_omega_power(std::move(inner)));
            }

            if (const auto* concatenation =
                    std::get_if<std::shared_ptr<const regex::omega::Concatenation>>(&expression))
            {
                regex::Expression prefix =
                    rewrite_finite_operand(require_node(*concatenation).prefix, options, alphabet);

                Rewritten suffix = enforce_removed_abbreviations(
                    require_node(*concatenation).suffix, options, alphabet
                );

                return checked(regex::omega::make_concatenation(
                    std::move(prefix), std::move(suffix.expression)
                ));
            }

            if (const auto* alternation =
                    std::get_if<std::shared_ptr<const regex::omega::Alternation>>(&expression))
            {
                std::vector<regex::omega::Expression> alternatives;
                std::size_t nodes = 1;

                for (const regex::omega::Expression& alternative :
                     require_node(*alternation).alternatives)
                {
                    Rewritten rewritten =
                        enforce_removed_abbreviations(alternative, options, alphabet);
                    add_nodes(nodes, rewritten.nodes);
                    alternatives.push_back(std::move(rewritten.expression));
                }

                return {regex::omega::make_alternation(std::move(alternatives)), nodes};
            }

            if (const auto* intersection =
                    std::get_if<std::shared_ptr<const regex::omega::Intersection>>(&expression))
            {
                std::vector<regex::omega::Expression> operands;
                std::size_t nodes = 1;

                for (const regex::omega::Expression& operand : require_node(*intersection).operands)
                {
                    Rewritten rewritten =
                        enforce_removed_abbreviations(operand, options, alphabet);
                    add_nodes(nodes, rewritten.nodes);
                    operands.push_back(std::move(rewritten.expression));
                }

                return {regex::omega::make_intersection(std::move(operands)), nodes};
            }

            const auto& complement =
                require_node(std::get<std::shared_ptr<const regex::omega::Complement>>(expression));

            Rewritten inner =
                enforce_removed_abbreviations(complement.expression, options, alphabet);
            add_nodes(inner.nodes, 1);

            return {
                regex::omega::make_complement(std::move(inner.expression)),
                inner.nodes
            };
        }

        spot::twa_graph_ptr postprocess(const spot::twa_graph_ptr& automaton)
        {
            spot::postprocessor postprocessor;
            postprocessor.set_type(spot::postprocessor::BA);
            postprocessor.set_pref(spot::postprocessor::Small);
            postprocessor.set_level(spot::postprocessor::Medium);
            return postprocessor.run(automaton);
        }

        Rewritten
        remove_omega_operator(const regex::omega::Expression& expression, const Alphabet& alphabet)
        {
            const regex::omega::Expression normalized =
                regex::simplification::omega::normalize(expression);

            const spot::twa_graph_ptr automaton =
                postprocess(conversion::to_spot_twa_raw(normalized, alphabet));

            return checked(conversion::to_omega_regex_ast(automaton, alphabet));
        }

        Rewritten rewrite_node(
            const regex::omega::Expression& expression,
            const Options& options,
            const Alphabet& alphabet
        )
        {
            if (std::holds_alternative<regex::omega::EmptySet>(expression))
            {
                return {expression, 1};
            }

            if (std::holds_alternative<regex::omega::UniversalSet>(expression))
            {
                if (!options.remove_any_symbol)
                {
                    return {expression, 1};
                }

                return checked(
                    regex::omega::make_omega_power(alphabet_expression(alphabet))
                );
            }

            if (const auto* power =
                    std::get_if<std::shared_ptr<const regex::omega::OmegaPower>>(&expression))
            {
                regex::Expression inner =
                    rewrite_finite_operand(require_node(*power).expression, options, alphabet);

                return checked(regex::omega::make_omega_power(std::move(inner)));
            }

            if (const auto* concatenation =
                    std::get_if<std::shared_ptr<const regex::omega::Concatenation>>(&expression))
            {
                regex::Expression prefix =
                    rewrite_finite_operand(require_node(*concatenation).prefix, options, alphabet);

                Rewritten suffix =
                    rewrite_node(require_node(*concatenation).suffix, options, alphabet);

                regex::omega::Expression result = regex::omega::make_concatenation(
                    std::move(prefix), std::move(suffix.expression)
                );

                return checked(std::move(result));
            }

            if (const auto* alternation =
                    std::get_if<std::shared_ptr<const regex::omega::Alternation>>(&expression))
            {
                std::vector<regex::omega::Expression> alternatives;
                std::size_t nodes = 1;

                for (const regex::omega::Expression& alternative :
                     require_node(*alternation).alternatives)
                {
                    Rewritten rewritten = rewrite_node(alternative, options, alphabet);
                    add_nodes(nodes, rewritten.nodes);
                    alternatives.push_back(std::move(rewritten.expression));
                }

                return {regex::omega::make_alternation(std::move(alternatives)), nodes};
            }

            if (const auto* intersection =
                    std::get_if<std::shared_ptr<const regex::omega::Intersection>>(&expression))
            {
                std::vector<regex::omega::Expression> operands;
                std::size_t nodes = 1;

                for (const regex::omega::Expression& operand : require_node(*intersection).operands)
                {
                    Rewritten rewritten = rewrite_node(operand, options, alphabet);
                    add_nodes(nodes, rewritten.nodes);
                    operands.push_back(std::move(rewritten.expression));
                }

                regex::omega::Expression rewritten =
                    regex::omega::make_intersection(std::move(operands));

                return options.remove_intersection ? remove_omega_operator(rewritten, alphabet)
                                                   : Rewritten{std::move(rewritten), nodes};
            }

            const auto& complement =
                require_node(std::get<std::shared_ptr<const regex::omega::Complement>>(expression));

            Rewritten inner = rewrite_node(complement.expression, options, alphabet);
            add_nodes(inner.nodes, 1);

            regex::omega::Expression rewritten =
                regex::omega::make_complement(std::move(inner.expression));

            return options.remove_complement ? remove_omega_operator(rewritten, alphabet)
                                             : Rewritten{std::move(rewritten), inner.nodes};
        }
    }

    std::string apply(
        const regex::omega::Expression& expression,
        const Options& options,
        const Alphabet& extra_alphabet
    )
    {
        const Alphabet alphabet = conversion::alphabet_of(expression, extra_alphabet);

        const regex::omega::Expression normalized =
            regex::simplification::omega::normalize(expression);
        checked(normalized);

        Rewritten rewritten = rewrite_node(normalized, options, alphabet);

        regex::omega::Expression result =
            regex::simplification::omega::normalize(rewritten.expression);
        checked(result);

        result = enforce_removed_abbreviations(result, options, alphabet).expression;
        checked(result);

        return regex::formatting::omega::format(result);
    }
}
