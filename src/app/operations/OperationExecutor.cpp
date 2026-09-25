// Implements validated dispatch for every operation request type.
#include "app/operations/OperationExecutor.hpp"

#include "app/operations/OperationValidation.hpp"
#include "automata/analysis/OmegaRegexComparison.hpp"
#include "automata/analysis/RegexComparison.hpp"
#include "automata/dot/BuchiDotExporter.hpp"
#include "automata/dot/DotExporter.hpp"
#include "automata/rewrite/OmegaRegexRewriter.hpp"
#include "automata/rewrite/RegexRewriter.hpp"
#include "automata/translation/OmegaRegexTranslation.hpp"
#include "automata/translation/RegexTranslation.hpp"
#include "graph/graphviz/GraphvizLayout.hpp"
#include "regex/inspection/ExpressionInspection.hpp"
#include "regex/inspection/OmegaExpressionInspection.hpp"
#include "regex/parsing/InputNormalization.hpp"
#include "regex/simplification/OmegaRegexSimplifier.hpp"
#include "regex/simplification/RegexSimplifier.hpp"

#include <array>
#include <exception>
#include <new>
#include <set>
#include <span>
#include <stdexcept>
#include <utility>
#include <variant>

namespace app::operations
{
    namespace
    {
        // Combines lambdas into one visitor for request dispatch.
        template <typename... Visitors>
        struct Overloaded : Visitors...
        {
            using Visitors::operator()...;
        };

        // Returns the alphabet supplied explicitly by the user plus finite terminals from parsed
        // finite expressions.
        [[nodiscard]] automata::Alphabet active_alphabet(automata::Alphabet alphabet,
            const std::span<const regex::Expression* const> expressions)
        {
            for (const regex::Expression* expression : expressions)
            {
                const std::set<char> terminals = regex::inspection::terminals(*expression);
                alphabet.insert(terminals.begin(), terminals.end());
            }

            return alphabet;
        }

        // Returns the alphabet supplied explicitly by the user plus finite terminals from parsed
        // omega expressions.
        [[nodiscard]] automata::Alphabet active_alphabet(automata::Alphabet alphabet,
            const std::span<const regex::omega::Expression* const> expressions)
        {
            for (const regex::omega::Expression* expression : expressions)
            {
                const std::set<char> terminals = regex::inspection::omega::terminals(*expression);
                alphabet.insert(terminals.begin(), terminals.end());
            }

            return alphabet;
        }

        // Computes a Graphviz layout for a checked DOT document.
        [[nodiscard]] graph::Layout layout_dot(const std::string& dot)
        {
            detail::require_dot_size(dot);

            graph::graphviz::LayoutResult layout_result = graph::graphviz::compute_layout(dot);
            if (const auto* error = std::get_if<graph::graphviz::LayoutError>(&layout_result))
            {
                throw std::runtime_error("Graph layout failed: " + error->message);
            }

            return std::get<graph::Layout>(std::move(layout_result));
        }

        // Validates, translates, bounds, and lays out a finite automaton request.
        [[nodiscard]] TranslateResult translate_finite(const TranslateRequest& request)
        {
            const regex::Expression parsed = detail::require_regex(request.regex, "Error");
            const automata::Alphabet extra_alphabet =
                detail::require_extra_alphabet(request.extra_alphabet);
            const std::array parsed_expressions{&parsed};
            detail::require_nonempty_alphabet(parsed_expressions, extra_alphabet);
            const automata::Alphabet alphabet =
                active_alphabet(extra_alphabet, std::span<const regex::Expression* const>(
                                                    parsed_expressions
                                                ));

            const regex::Expression expression = regex::simplification::normalize(parsed);

            const bool use_dfa = request.mode == TranslateMode::Dfa;
            const automata::Nfa automaton =
                use_dfa ? automata::translation::minimal_dfa(expression, alphabet)
                        : automata::translation::epsilon_free_nfa(expression, alphabet);
            detail::require_renderable(automaton);

            const std::string dot = automata::dot::to_dot(automaton);

            return TranslateResult{layout_dot(dot)};
        }

        // Constructs a tiny Büchi automaton for the empty omega language.
        [[nodiscard]] automata::BuchiAutomaton empty_omega_automaton(
            automata::Alphabet alphabet = {}
        )
        {
            automata::BuchiAutomaton automaton;
            automaton.start = 0;
            automaton.transitions.emplace_back();
            automaton.alphabet = std::move(alphabet);
            return automaton;
        }

        // Validates, translates, bounds, and lays out an omega automaton request.
        [[nodiscard]] OperationResult translate_omega(const TranslateRequest& request)
        {
            const regex::omega::Expression parsed =
                detail::require_omega_regex(request.regex, "Error");
            const automata::Alphabet extra_alphabet =
                detail::require_extra_alphabet(request.extra_alphabet);
            const std::array parsed_expressions{&parsed};
            detail::require_nonempty_alphabet(parsed_expressions, extra_alphabet);
            const automata::Alphabet alphabet = active_alphabet(
                extra_alphabet,
                std::span<const regex::omega::Expression* const>(parsed_expressions)
            );

            const regex::omega::Expression expression =
                regex::simplification::omega::normalize(parsed);

            automata::BuchiAutomaton automaton;
            if (std::holds_alternative<regex::omega::EmptySet>(expression))
            {
                automaton = empty_omega_automaton(alphabet);
            }
            else if (request.mode == TranslateMode::Dfa)
            {
                auto deterministic = automata::translation::minimal_dba(expression, alphabet);
                if (!deterministic)
                {
                    return OperationFailure{
                        "No deterministic Büchi automaton (DBA) exists for this omega regular "
                        "expression. Use NBA mode to display its language."
                    };
                }
                automaton = std::move(*deterministic);
            }
            else
            {
                automaton = automata::translation::buchi_automaton(expression, alphabet);
            }

            detail::require_renderable(automaton);
            const std::string dot = automata::dot::to_dot(automaton);

            return TranslateResult{layout_dot(dot)};
        }

        // Validates, translates, bounds, and lays out an automaton request.
        [[nodiscard]] OperationResult translate(const TranslateRequest& request)
        {
            if (request.flavor == ExpressionFlavor::OmegaRegex)
            {
                return translate_omega(request);
            }

            return translate_finite(request);
        }

        // Creates rewrite options from the request checkboxes.
        [[nodiscard]] automata::rewrite::Options rewrite_options(const RewriteRequest& request)
        {
            automata::rewrite::Options options;
            options.remove_complement = request.remove_complement;
            options.remove_intersection = request.remove_intersection;
            options.remove_power = request.remove_power;
            options.remove_plus = request.remove_plus;
            options.remove_any_symbol = request.remove_any_symbol;
            return options;
        }

        // Validates and applies finite regular-expression rewrites.
        [[nodiscard]] RewriteResult rewrite_finite(const RewriteRequest& request)
        {
            const regex::Expression expression = detail::require_regex(request.regex, "Error");
            const automata::Alphabet extra_alphabet =
                detail::require_extra_alphabet(request.extra_alphabet);
            const std::array expressions{&expression};
            detail::require_nonempty_alphabet(expressions, extra_alphabet);
            const automata::Alphabet alphabet = active_alphabet(
                extra_alphabet,
                std::span<const regex::Expression* const>(expressions)
            );

            return RewriteResult{
                automata::rewrite::apply(expression, rewrite_options(request), alphabet)
            };
        }

        // Validates and applies omega regular-expression rewrites.
        [[nodiscard]] RewriteResult rewrite_omega(const RewriteRequest& request)
        {
            const regex::omega::Expression expression =
                detail::require_omega_regex(request.regex, "Error");
            const automata::Alphabet extra_alphabet =
                detail::require_extra_alphabet(request.extra_alphabet);
            const std::array expressions{&expression};
            detail::require_nonempty_alphabet(expressions, extra_alphabet);
            const automata::Alphabet alphabet = active_alphabet(
                extra_alphabet,
                std::span<const regex::omega::Expression* const>(expressions)
            );

            return RewriteResult{automata::rewrite::omega::apply(
                expression, rewrite_options(request), alphabet
            )};
        }

        // Validates and applies the requested bounded expression expansions.
        [[nodiscard]] RewriteResult rewrite(const RewriteRequest& request)
        {
            if (request.flavor == ExpressionFlavor::OmegaRegex)
            {
                return rewrite_omega(request);
            }

            return rewrite_finite(request);
        }

        // Validates two finite expressions and compares them over one merged alphabet.
        [[nodiscard]] CompareResult compare_finite(const CompareRequest& request)
        {
            if (regex::parsing::normalize_input(request.left_regex).empty() ||
                regex::parsing::normalize_input(request.right_regex).empty())
            {
                throw std::runtime_error("Error: Please enter both regular expressions.");
            }

            const regex::Expression parsed_left =
                detail::require_regex(request.left_regex, "Error in first regex");
            const regex::Expression parsed_right =
                detail::require_regex(request.right_regex, "Error in second regex");
            const automata::Alphabet extra_alphabet =
                detail::require_extra_alphabet(request.extra_alphabet);
            const std::array parsed_expressions{&parsed_left, &parsed_right};
            detail::require_nonempty_alphabet(parsed_expressions, extra_alphabet);
            const automata::Alphabet alphabet = active_alphabet(
                extra_alphabet,
                std::span<const regex::Expression* const>(parsed_expressions)
            );

            const regex::Expression left = regex::simplification::normalize(parsed_left);
            const regex::Expression right = regex::simplification::normalize(parsed_right);

            return CompareResult{automata::analysis::compare(left, right, alphabet)};
        }

        // Validates two omega expressions and compares them over one merged alphabet.
        [[nodiscard]] CompareResult compare_omega(const CompareRequest& request)
        {
            if (regex::parsing::normalize_input(request.left_regex).empty() ||
                regex::parsing::normalize_input(request.right_regex).empty())
            {
                throw std::runtime_error("Error: Please enter both omega regular expressions.");
            }

            const regex::omega::Expression parsed_left =
                detail::require_omega_regex(request.left_regex, "Error in first omega regex");
            const regex::omega::Expression parsed_right =
                detail::require_omega_regex(request.right_regex, "Error in second omega regex");
            const automata::Alphabet extra_alphabet =
                detail::require_extra_alphabet(request.extra_alphabet);
            const std::array parsed_expressions{&parsed_left, &parsed_right};
            detail::require_nonempty_alphabet(parsed_expressions, extra_alphabet);
            const automata::Alphabet alphabet = active_alphabet(
                extra_alphabet,
                std::span<const regex::omega::Expression* const>(parsed_expressions)
            );

            const regex::omega::Expression left =
                regex::simplification::omega::normalize(parsed_left);
            const regex::omega::Expression right =
                regex::simplification::omega::normalize(parsed_right);

            return CompareResult{automata::analysis::compare(left, right, alphabet)};
        }

        // Validates two expressions and compares them over one merged alphabet.
        [[nodiscard]] CompareResult compare(const CompareRequest& request)
        {
            if (request.flavor == ExpressionFlavor::OmegaRegex)
            {
                return compare_omega(request);
            }

            return compare_finite(request);
        }

    }

    OperationResult execute(OperationRequest request)
    {
        try
        {
            return std::visit(
                Overloaded{
                    [](const TranslateRequest& value) -> OperationResult
                    { return translate(value); },
                    [](const RewriteRequest& value) -> OperationResult { return rewrite(value); },
                    [](const CompareRequest& value) -> OperationResult { return compare(value); }
                },
                request
            );
        }
        catch (const std::bad_alloc&)
        {
            return OperationFailure{
                "The operation ran out of memory. Try a smaller regular expression."
            };
        }
        catch (const std::exception& exception)
        {
            return OperationFailure{exception.what()};
        }
        catch (...)
        {
            return OperationFailure{"An unknown error occurred while running the operation."};
        }
    }
}
