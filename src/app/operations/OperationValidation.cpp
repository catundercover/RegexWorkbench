// Implements user-input and graph-complexity validation for operations.
#include "app/operations/OperationValidation.hpp"

#include "app/operations/OperationLimits.hpp"
#include "regex/inspection/ExpressionInspection.hpp"
#include "regex/inspection/OmegaExpressionInspection.hpp"
#include "regex/parsing/InputNormalization.hpp"
#include "regex/parsing/OmegaRegexParser.hpp"
#include "regex/parsing/RegexParser.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <variant>

namespace app::operations
{
    AlphabetParseResult parse_extra_alphabet(const std::string_view input)
    {
        AlphabetParseResult result;

        for (const char character : input)
        {
            if (character == ' ' || character == '\t' || character == '\n' || character == '\r' ||
                character == '\f' || character == '\v')
            {
                continue;
            }

            if (!regex::is_ascii_terminal(character))
            {
                result.symbols.clear();
                result.error = "Only alphanumerical symbols are valid terminals.";
                return result;
            }

            result.symbols.insert(character);
        }

        return result;
    }

    namespace detail
    {
        namespace
        {
            // Combines the expression-derived and explicitly supplied symbols.
            [[nodiscard]] bool active_alphabet_empty(const std::set<char>& expression_alphabet,
                const automata::Alphabet& extra_alphabet)
            {
                return expression_alphabet.empty() && extra_alphabet.empty();
            }

            // Returns whether a finite regex needs a concrete alphabet to be meaningful.
            [[nodiscard]] bool references_alphabet(const regex::Expression& expression)
            {
                if (regex::inspection::contains_any_symbol(expression))
                {
                    return true;
                }

                return std::visit(
                    [](const auto& node) -> bool
                    {
                        using Node = std::decay_t<decltype(node)>;

                        if constexpr (std::is_same_v<Node, regex::Terminal> ||
                                      std::is_same_v<Node, regex::Epsilon> ||
                                      std::is_same_v<Node, regex::EmptySet> ||
                                      std::is_same_v<Node, regex::AnySymbol>)
                        {
                            return false;
                        }
                        else if constexpr (std::is_same_v<
                                               Node,
                                               std::shared_ptr<const regex::Complement>>)
                        {
                            return true;
                        }
                        else if constexpr (std::is_same_v<
                                               Node,
                                               std::shared_ptr<const regex::KleeneStar>> ||
                                           std::is_same_v<
                                               Node,
                                               std::shared_ptr<const regex::Plus>> ||
                                           std::
                                               is_same_v<Node, std::shared_ptr<const regex::Power>>)
                        {
                            return node && references_alphabet(node->expression);
                        }
                        else if constexpr (std::is_same_v<
                                               Node,
                                               std::shared_ptr<const regex::Alternation>>)
                        {
                            return node && std::any_of(
                                               node->alternatives.begin(),
                                               node->alternatives.end(),
                                               [](const regex::Expression& child)
                                               { return references_alphabet(child); }
                                           );
                        }
                        else if constexpr (std::is_same_v<
                                               Node,
                                               std::shared_ptr<const regex::Intersection>>)
                        {
                            return node && std::any_of(
                                               node->operands.begin(),
                                               node->operands.end(),
                                               [](const regex::Expression& child)
                                               { return references_alphabet(child); }
                                           );
                        }
                        else
                        {
                            return node && std::any_of(
                                               node->parts.begin(),
                                               node->parts.end(),
                                               [](const regex::Expression& child)
                                               { return references_alphabet(child); }
                                           );
                        }
                    },
                    expression
                    );
                }

                // Returns whether an omega regex needs a concrete alphabet to be meaningful.
                [[nodiscard]] bool references_alphabet(const regex::omega::Expression& expression)
                {
                    if (regex::inspection::omega::contains_any_symbol(expression))
                    {
                        return true;
                    }

                    return std::visit(
                        [](const auto& node) -> bool
                        {
                            using Node = std::decay_t<decltype(node)>;

                            if constexpr (std::is_same_v<Node, regex::omega::EmptySet>)
                            {
                                return false;
                            }
                            else if constexpr (std::is_same_v<Node, regex::omega::UniversalSet>)
                            {
                                return true;
                            }
                            else if constexpr (
                                std::is_same_v<Node, std::shared_ptr<const regex::omega::OmegaPower>>)
                            {
                                return node && references_alphabet(node->expression);
                            }
                            else if constexpr (std::is_same_v<Node, std::shared_ptr<const regex::omega::Concatenation>>)
                            {
                                return node &&
                                       (references_alphabet(node->prefix) ||
                                        references_alphabet(node->suffix));
                            }
                            else if constexpr (std::is_same_v<Node, std::shared_ptr<const regex::omega::Complement>>)
                            {
                                return true;
                            }
                            else if constexpr (std::is_same_v<Node, std::shared_ptr<const regex::omega::Alternation>>)
                            {
                                return node && std::any_of(
                                                   node->alternatives.begin(),
                                                   node->alternatives.end(),
                                                   [](const regex::omega::Expression& child)
                                                   { return references_alphabet(child); }
                                               );
                            }
                            else
                            {
                                return node && std::any_of(
                                                   node->operands.begin(),
                                                   node->operands.end(),
                                                   [](const regex::omega::Expression& child)
                                                   { return references_alphabet(child); }
                                               );
                            }
                        },
                        expression
                    );
                }

                // Formats a finite parser result as a labeled, source-aware user diagnostic.
                [[nodiscard]] std::string parse_error_message(
                    const std::string_view prefix, const regex::parsing::ParseResult& result
                )
            {
                std::ostringstream out;
                if (result.error)
                {
                    out << prefix << " at line " << result.error->line << ", column "
                        << result.error->column << ": " << result.error->message;
                }
                else
                {
                    out << prefix << ": invalid syntax.";
                }
                return out.str();
            }

            // Formats an omega parser result as a labeled, source-aware user diagnostic.
            [[nodiscard]] std::string parse_error_message(
                const std::string_view prefix, const regex::parsing::omega::ParseResult& result)
            {
                std::ostringstream out;
                if (result.error)
                {
                    out << prefix << " at line " << result.error->line << ", column "
                        << result.error->column << ": " << result.error->message;
                }
                else
                {
                    out << prefix << ": invalid omega regular-expression syntax.";
                }
                return out.str();
            }
        }

        regex::Expression
        require_regex(const std::string_view raw_regex, const std::string_view label)
        {
            if (raw_regex.size() > limits::MaxRegexInputBytes)
            {
                std::ostringstream out;
                out << "The regular expression is too long. Maximum length: "
                    << limits::MaxRegexInputBytes << " bytes.";
                throw std::runtime_error(out.str());
            }

            if (regex::parsing::normalize_input(raw_regex).empty())
            {
                throw std::runtime_error(
                    std::string(label) + ": Please enter a regular expression."
                );
            }

            regex::parsing::ParseResult parsed = regex::parsing::parse(raw_regex);
            if (!parsed.expression.has_value() || parsed.error.has_value())
            {
                throw std::runtime_error(parse_error_message(label, parsed));
            }
            return std::move(parsed.expression).value();
        }

        regex::omega::Expression
        require_omega_regex(const std::string_view raw_regex, const std::string_view label)
        {
            if (raw_regex.size() > limits::MaxRegexInputBytes)
            {
                std::ostringstream out;
                out << "The omega regular expression is too long. Maximum length: "
                    << limits::MaxRegexInputBytes << " bytes.";
                throw std::runtime_error(out.str());
            }

            if (regex::parsing::normalize_input(raw_regex).empty())
            {
                throw std::runtime_error(
                    std::string(label) + ": Please enter an omega regular expression."
                );
            }

            regex::parsing::omega::ParseResult parsed = regex::parsing::omega::parse(raw_regex);
            if (!parsed.expression.has_value() || parsed.error.has_value())
            {
                throw std::runtime_error(parse_error_message(label, parsed));
            }
            return std::move(parsed.expression).value();
        }

        automata::Alphabet require_extra_alphabet(const std::string_view input)
        {
            AlphabetParseResult parsed = parse_extra_alphabet(input);
            if (parsed.error)
            {
                throw std::runtime_error("Error: " + *parsed.error);
            }
            return std::move(parsed.symbols);
        }

        void require_nonempty_alphabet(const std::span<const regex::Expression* const> expressions,
            const automata::Alphabet& extra_alphabet)
        {
            std::set<char> expression_alphabet;
            bool alphabet_dependent = false;

            for (const regex::Expression* expression : expressions)
            {
                const std::set<char> expression_terminals =
                    regex::inspection::terminals(*expression);
                expression_alphabet.insert(
                    expression_terminals.begin(), expression_terminals.end()
                );
                alphabet_dependent = alphabet_dependent || references_alphabet(*expression);
            }

            if (active_alphabet_empty(expression_alphabet, extra_alphabet) && alphabet_dependent)
            {
                throw std::runtime_error("Error: Alphabet is empty");
            }
        }

        void require_nonempty_alphabet(const std::span<const regex::omega::Expression* const> expressions,
            const automata::Alphabet& extra_alphabet)
        {
            std::set<char> expression_alphabet;
            bool alphabet_dependent = false;

            for (const regex::omega::Expression* expression : expressions)
            {
                const std::set<char> expression_terminals =
                    regex::inspection::omega::terminals(*expression);
                expression_alphabet.insert(
                    expression_terminals.begin(), expression_terminals.end()
                );
                alphabet_dependent = alphabet_dependent || references_alphabet(*expression);
            }

            if (active_alphabet_empty(expression_alphabet, extra_alphabet) && alphabet_dependent)
            {
                throw std::runtime_error("Error: Alphabet is empty");
            }
        }

        std::size_t rendered_transition_count(const automata::Nfa& automaton)
        {
            std::size_t count = 1; // The synthetic start-marker edge.
            for (const auto& outgoing : automaton.transitions)
            {
                std::unordered_set<automata::StateId> targets;
                for (const automata::Transition& transition : outgoing)
                {
                    targets.insert(transition.target);
                }
                count += targets.size();
            }
            return count;
        }

        std::size_t rendered_transition_count(const automata::BuchiAutomaton& automaton)
        {
            std::size_t count = 1; // The synthetic start-marker edge.
            for (const auto& outgoing : automaton.transitions)
            {
                std::unordered_set<automata::StateId> targets;
                for (const automata::BuchiTransition& transition : outgoing)
                {
                    targets.insert(transition.target);
                }
                count += targets.size();
            }
            return count;
        }

        void require_renderable(const automata::Nfa& automaton)
        {
            const std::size_t state_count = automaton.transitions.size();
            if (state_count > limits::MaxRenderedStates)
            {
                std::ostringstream out;
                out << "The automaton has too many states to render. States: " << state_count
                    << ", maximum: " << limits::MaxRenderedStates
                    << ". Try NFA mode or a smaller expression.";
                throw std::runtime_error(out.str());
            }

            const std::size_t transition_count = rendered_transition_count(automaton);
            if (transition_count > limits::MaxRenderedTransitions)
            {
                std::ostringstream out;
                out << "The automaton has too many transitions to render. Transitions: "
                    << transition_count << ", maximum: " << limits::MaxRenderedTransitions
                    << ". Try NFA mode or a smaller expression.";
                throw std::runtime_error(out.str());
            }
        }

        void require_renderable(const automata::BuchiAutomaton& automaton)
        {
            const std::size_t state_count = automaton.transitions.size();
            if (state_count > limits::MaxRenderedStates)
            {
                std::ostringstream out;
                out << "The Büchi automaton has too many states to render. States: " << state_count
                    << ", maximum: " << limits::MaxRenderedStates
                    << ". Try a smaller omega regular expression.";
                throw std::runtime_error(out.str());
            }

            const std::size_t transition_count = rendered_transition_count(automaton);
            if (transition_count > limits::MaxRenderedTransitions)
            {
                std::ostringstream out;
                out << "The Büchi automaton has too many transitions to render. Transitions: "
                    << transition_count << ", maximum: " << limits::MaxRenderedTransitions
                    << ". Try a smaller omega regular expression.";
                throw std::runtime_error(out.str());
            }
        }

        void require_dot_size(const std::string_view dot)
        {
            if (dot.size() <= limits::MaxDotBytes)
            {
                return;
            }

            std::ostringstream out;
            out << "The generated graph is too large to display. DOT size: " << dot.size()
                << " bytes, maximum: " << limits::MaxDotBytes << " bytes.";
            throw std::runtime_error(out.str());
        }
    }
}
