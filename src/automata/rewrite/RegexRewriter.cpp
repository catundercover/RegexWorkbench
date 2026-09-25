// Implements bounded expansion of optional regex operators.
#include "automata/rewrite/RegexRewriter.hpp"

#include "automata/conversion/NfaToRegex.hpp"
#include "automata/conversion/RegexAlphabet.hpp"
#include "automata/conversion/RegexToNfa.hpp"
#include "regex/formatting/RegexFormatter.hpp"
#include "regex/inspection/ExpressionInspection.hpp"
#include "regex/simplification/RegexSimplifier.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

namespace automata::rewrite
{
    namespace
    {
        // Carries a rewritten subtree together with its precomputed node count.
        struct Rewritten
        {
            regex::Expression expression;
            std::size_t nodes;
        };

        // Throws the consistent diagnostic used for every output-budget violation.
        [[noreturn]] void throw_budget_error()
        {
            throw std::runtime_error(
                "Rewriting would exceed the limit of " + std::to_string(MaxOutputNodes) +
                " expression nodes. Use a smaller exponent or keep the abbreviation."
            );
        }

        // Counts and validates a complete expression against the output budget.
        Rewritten checked(regex::Expression expression)
        {
            const std::size_t nodes = regex::inspection::node_count(expression);
            if (nodes > MaxOutputNodes)
            {
                throw_budget_error();
            }
            return {std::move(expression), nodes};
        }

        // Adds to a node count without overflow or budget overrun.
        void add_nodes(std::size_t& total, const std::size_t additional)
        {
            if (additional > MaxOutputNodes || total > MaxOutputNodes - additional)
            {
                throw_budget_error();
            }
            total += additional;
        }

        // Dereferences a recursive AST node or rejects an invalid null pointer.
        template <typename Node>
        const Node& require_node(const std::shared_ptr<const Node>& node)
        {
            if (!node)
            {
                throw std::invalid_argument("Invalid null node during regex rewrite");
            }
            return *node;
        }

        // Expands Sigma into a deterministic alternation of alphabet symbols.
        regex::Expression any_symbol_expression(const Alphabet& alphabet)
        {
            if (alphabet.empty())
            {
                throw std::invalid_argument("Cannot expand Sigma over an empty alphabet");
            }

            std::vector<Symbol> symbols(alphabet.begin(), alphabet.end());
            std::sort(symbols.begin(), symbols.end());
            std::vector<regex::Expression> alternatives;
            alternatives.reserve(symbols.size());
            for (const Symbol symbol : symbols)
            {
                alternatives.push_back(regex::make_terminal(symbol));
            }
            return regex::make_alternation(std::move(alternatives));
        }

        // Removes a language-level operator through automaton conversion.
        Rewritten
        remove_automata_operator(const regex::Expression& expression, const Alphabet& alphabet)
        {
            // This is deliberately the only helper that constructs an automaton during rewrite.
            return checked(conversion::to_regex_ast(conversion::to_nfa(expression, alphabet)));
        }

        // Final safety pass for operators that may have been reintroduced by simplification.
        Rewritten enforce_removed_abbreviations(
            const regex::Expression& expression,
            const Options& options,
            const Alphabet& alphabet
        )
        {
            if (std::holds_alternative<regex::Terminal>(expression) ||
                std::holds_alternative<regex::Epsilon>(expression) ||
                std::holds_alternative<regex::EmptySet>(expression))
            {
                return {expression, 1};
            }

            if (std::holds_alternative<regex::AnySymbol>(expression))
            {
                return options.remove_any_symbol ? checked(any_symbol_expression(alphabet))
                                                 : Rewritten{expression, 1};
            }

            if (const auto* star =
                    std::get_if<std::shared_ptr<const regex::KleeneStar>>(&expression))
            {
                Rewritten inner =
                    enforce_removed_abbreviations(require_node(*star).expression, options, alphabet);
                add_nodes(inner.nodes, 1);
                return {regex::make_star(std::move(inner.expression)), inner.nodes};
            }

            if (const auto* plus = std::get_if<std::shared_ptr<const regex::Plus>>(&expression))
            {
                Rewritten inner =
                    enforce_removed_abbreviations(require_node(*plus).expression, options, alphabet);
                add_nodes(inner.nodes, 1);
                return {regex::make_plus(std::move(inner.expression)), inner.nodes};
            }

            if (const auto* power = std::get_if<std::shared_ptr<const regex::Power>>(&expression))
            {
                const regex::Power& power_node = require_node(*power);
                Rewritten inner =
                    enforce_removed_abbreviations(power_node.expression, options, alphabet);
                add_nodes(inner.nodes, 1);
                return {
                    regex::make_power(std::move(inner.expression), power_node.exponent),
                    inner.nodes
                };
            }

            if (const auto* concatenation =
                    std::get_if<std::shared_ptr<const regex::Concatenation>>(&expression))
            {
                std::vector<regex::Expression> parts;
                std::size_t nodes = 1;

                for (const regex::Expression& part : require_node(*concatenation).parts)
                {
                    Rewritten rewritten =
                        enforce_removed_abbreviations(part, options, alphabet);
                    add_nodes(nodes, rewritten.nodes);
                    parts.push_back(std::move(rewritten.expression));
                }

                return {regex::make_concatenation(std::move(parts)), nodes};
            }

            if (const auto* alternation =
                    std::get_if<std::shared_ptr<const regex::Alternation>>(&expression))
            {
                std::vector<regex::Expression> alternatives;
                std::size_t nodes = 1;

                for (const regex::Expression& alternative :
                     require_node(*alternation).alternatives)
                {
                    Rewritten rewritten =
                        enforce_removed_abbreviations(alternative, options, alphabet);
                    add_nodes(nodes, rewritten.nodes);
                    alternatives.push_back(std::move(rewritten.expression));
                }

                return {regex::make_alternation(std::move(alternatives)), nodes};
            }

            if (const auto* intersection =
                    std::get_if<std::shared_ptr<const regex::Intersection>>(&expression))
            {
                std::vector<regex::Expression> operands;
                std::size_t nodes = 1;

                for (const regex::Expression& operand : require_node(*intersection).operands)
                {
                    Rewritten rewritten =
                        enforce_removed_abbreviations(operand, options, alphabet);
                    add_nodes(nodes, rewritten.nodes);
                    operands.push_back(std::move(rewritten.expression));
                }

                return {regex::make_intersection(std::move(operands)), nodes};
            }

            const auto& complement =
                require_node(std::get<std::shared_ptr<const regex::Complement>>(expression));

            Rewritten inner =
                enforce_removed_abbreviations(complement.expression, options, alphabet);
            add_nodes(inner.nodes, 1);

            return {regex::make_complement(std::move(inner.expression)), inner.nodes};
        }

        // Recursively applies selected expansions while tracking the output budget.
        Rewritten rewrite_node(
            const regex::Expression& expression, const Options& options, const Alphabet& alphabet
        )
        {
            if (std::holds_alternative<regex::Terminal>(expression) ||
                std::holds_alternative<regex::Epsilon>(expression) ||
                std::holds_alternative<regex::EmptySet>(expression))
            {
                return {expression, 1};
            }
            if (std::holds_alternative<regex::AnySymbol>(expression))
            {
                return options.remove_any_symbol ? checked(any_symbol_expression(alphabet))
                                                 : Rewritten{expression, 1};
            }
            if (const auto* star =
                    std::get_if<std::shared_ptr<const regex::KleeneStar>>(&expression))
            {
                Rewritten inner = rewrite_node(require_node(*star).expression, options, alphabet);
                add_nodes(inner.nodes, 1);
                return {regex::make_star(std::move(inner.expression)), inner.nodes};
            }
            if (const auto* plus = std::get_if<std::shared_ptr<const regex::Plus>>(&expression))
            {
                Rewritten inner = rewrite_node(require_node(*plus).expression, options, alphabet);
                if (!options.remove_plus)
                {
                    add_nodes(inner.nodes, 1);
                    return {regex::make_plus(std::move(inner.expression)), inner.nodes};
                }

                const regex::Expression repeated = inner.expression;
                std::size_t output_nodes = 2;
                add_nodes(output_nodes, inner.nodes);
                add_nodes(output_nodes, inner.nodes);
                return {
                    regex::make_concatenation(
                        {std::move(inner.expression), regex::make_star(repeated)}
                    ),
                    output_nodes
                };
            }
            if (const auto* power = std::get_if<std::shared_ptr<const regex::Power>>(&expression))
            {
                const regex::Power& power_node = require_node(*power);
                Rewritten inner = rewrite_node(power_node.expression, options, alphabet);
                if (!options.remove_power)
                {
                    add_nodes(inner.nodes, 1);
                    return {
                        regex::make_power(std::move(inner.expression), power_node.exponent),
                        inner.nodes
                    };
                }
                if (power_node.exponent == 0)
                {
                    return {regex::Epsilon{}, 1};
                }
                if (power_node.exponent == 1)
                {
                    return inner;
                }
                if (inner.nodes > (MaxOutputNodes - 1) / power_node.exponent)
                {
                    throw_budget_error();
                }

                std::vector<regex::Expression> parts;
                parts.reserve(power_node.exponent);
                for (unsigned int exponent = 0; exponent < power_node.exponent; ++exponent)
                {
                    parts.push_back(inner.expression);
                }
                return {
                    regex::make_concatenation(std::move(parts)),
                    1 + inner.nodes * power_node.exponent
                };
            }
            if (const auto* concatenation =
                    std::get_if<std::shared_ptr<const regex::Concatenation>>(&expression))
            {
                std::vector<regex::Expression> parts;
                std::size_t nodes = 1;
                for (const regex::Expression& part : require_node(*concatenation).parts)
                {
                    Rewritten rewritten = rewrite_node(part, options, alphabet);
                    add_nodes(nodes, rewritten.nodes);
                    parts.push_back(std::move(rewritten.expression));
                }
                return {regex::make_concatenation(std::move(parts)), nodes};
            }
            if (const auto* alternation =
                    std::get_if<std::shared_ptr<const regex::Alternation>>(&expression))
            {
                std::vector<regex::Expression> alternatives;
                std::size_t nodes = 1;
                for (const regex::Expression& alternative : require_node(*alternation).alternatives)
                {
                    Rewritten rewritten = rewrite_node(alternative, options, alphabet);
                    add_nodes(nodes, rewritten.nodes);
                    alternatives.push_back(std::move(rewritten.expression));
                }
                return {regex::make_alternation(std::move(alternatives)), nodes};
            }
            if (const auto* intersection =
                    std::get_if<std::shared_ptr<const regex::Intersection>>(&expression))
            {
                std::vector<regex::Expression> operands;
                std::size_t nodes = 1;
                for (const regex::Expression& operand : require_node(*intersection).operands)
                {
                    Rewritten rewritten = rewrite_node(operand, options, alphabet);
                    add_nodes(nodes, rewritten.nodes);
                    operands.push_back(std::move(rewritten.expression));
                }
                regex::Expression rewritten = regex::make_intersection(std::move(operands));
                return options.remove_intersection ? remove_automata_operator(rewritten, alphabet)
                                                   : Rewritten{std::move(rewritten), nodes};
            }

            const auto& complement =
                require_node(std::get<std::shared_ptr<const regex::Complement>>(expression));
            Rewritten inner = rewrite_node(complement.expression, options, alphabet);
            add_nodes(inner.nodes, 1);
            regex::Expression rewritten = regex::make_complement(std::move(inner.expression));
            return options.remove_complement ? remove_automata_operator(rewritten, alphabet)
                                             : Rewritten{std::move(rewritten), inner.nodes};
        }
    }

    std::string apply(
       const regex::Expression& expression, const Options& options, const Alphabet& extra_alphabet
   )
    {
        const Alphabet alphabet = conversion::alphabet_of(expression, extra_alphabet);
        const regex::Expression normalized = regex::simplification::normalize(expression);
        checked(normalized);
        Rewritten rewritten = rewrite_node(normalized, options, alphabet);
        regex::Expression result = regex::simplification::normalize(rewritten.expression);
        checked(result);
        result = enforce_removed_abbreviations(result, options, alphabet).expression;
        checked(result);
        return regex::formatting::format(result);
    }
}
