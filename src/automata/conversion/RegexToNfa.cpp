// Implements recursive Thompson construction and MATA-backed operators.
#include "automata/conversion/RegexToNfa.hpp"

#include "automata/conversion/MataAdapter.hpp"
#include "automata/conversion/RegexAlphabet.hpp"
#include "regex/simplification/RegexSimplifier.hpp"

#include <stdexcept>
#include <utility>
#include <vector>
#include <algorithm>

namespace automata::conversion
{
    namespace
    {
        std::size_t transition_count(const mata::nfa::Nfa& automaton)
        {
            std::size_t count = 0;
            for (mata::nfa::State state = 0; state < automaton.num_of_states(); ++state)
            {
                for (const auto& transition : automaton.delta[state])
                {
                    count += transition.targets.size();
                }
            }
            return count;
        }

        std::size_t automaton_size_score(const mata::nfa::Nfa& automaton)
        {
            return automaton.num_of_states() + transition_count(automaton);
        }

        bool is_large_intermediate(const mata::nfa::Nfa& automaton)
        {
            return automaton.num_of_states() >= 64 || transition_count(automaton) >= 512;
        }

        mata::nfa::Nfa maybe_minimize_large(mata::nfa::Nfa automaton)
        {
            automaton = automaton.trim();
            if (is_large_intermediate(automaton))
            {
                return detail::minimize(automaton).trim();
            }
            return automaton;
        }

        // Identifies the entry and accepting state of a Thompson fragment.
        struct Fragment
        {
            StateId start;
            StateId accept;
        };

        // Appends a two-state fragment that optionally accepts epsilon.
        Fragment empty_fragment(Nfa& nfa, bool accepts_epsilon)
        {
            const StateId start = nfa.add_state();
            const StateId accept = nfa.add_state();
            if (accepts_epsilon)
            {
                nfa.add_epsilon_transition(start, accept);
            }
            return {start, accept};
        }

        // Copies an automaton into a target NFA behind one-entry, one-exit wrapper states.
        Fragment append(const Nfa& source, Nfa& target)
        {
            if (!source.is_valid())
            {
                throw std::invalid_argument("Cannot append an invalid NFA");
            }

            const StateId fragment_start = target.add_state();
            const StateId fragment_accept = target.add_state();
            std::vector<StateId> state_map(source.transitions.size());

            for (StateId state = 0; state < source.transitions.size(); ++state)
            {
                state_map[state] = target.add_state();
            }

            target.add_epsilon_transition(fragment_start, state_map[source.start]);
            for (StateId from = 0; from < source.transitions.size(); ++from)
            {
                for (const Transition& transition : source.transitions[from])
                {
                    if (transition.symbol)
                    {
                        target.add_transition(
                            state_map[from], *transition.symbol, state_map[transition.target]
                        );
                    }
                    else
                    {
                        target.add_epsilon_transition(
                            state_map[from], state_map[transition.target]
                        );
                    }
                }
            }

            for (StateId final_state : source.finals)
            {
                target.add_epsilon_transition(state_map[final_state], fragment_accept);
            }

            return {fragment_start, fragment_accept};
        }

        mata::nfa::Nfa intersect_all(std::vector<mata::nfa::Nfa> automata)
        {
            if (automata.empty())
            {
                    return mata::nfa::Nfa{};
            }

            std::ranges::sort(
                automata,
                [](const mata::nfa::Nfa& left, const mata::nfa::Nfa& right)
                {
                    return automaton_size_score(left) < automaton_size_score(right);
                }
            );

            while (automata.size() > 1)
            {
                std::vector<mata::nfa::Nfa> next;
                next.reserve((automata.size() + 1) / 2);

                for (std::size_t index = 0; index < automata.size(); index += 2)
                {
                    if (index + 1 == automata.size())
                    {
                        next.push_back(std::move(automata[index]));
                        continue;
                    }

                    next.push_back(
                        maybe_minimize_large(
                            mata::nfa::intersection(automata[index], automata[index + 1])
                        )
                    );
                }

                automata = std::move(next);
                std::ranges::sort(
                    automata,
                    [](const mata::nfa::Nfa& left, const mata::nfa::Nfa& right)
                    {
                        return automaton_size_score(left) < automaton_size_score(right);
                    }
                );
            }

            return automata.front();
        }

        // Recursively appends the Thompson fragment for one expression node.
        Fragment build(const regex::Expression& node, Nfa& nfa, const Alphabet& alphabet)
        {
            if (const auto* terminal = std::get_if<regex::Terminal>(&node))
            {
                const Fragment result = empty_fragment(nfa, false);
                nfa.add_transition(result.start, terminal->value, result.accept);
                return result;
            }

            if (std::holds_alternative<regex::Epsilon>(node))
            {
                return empty_fragment(nfa, true);
            }

            if (std::holds_alternative<regex::EmptySet>(node))
            {
                return empty_fragment(nfa, false);
            }

            if (std::holds_alternative<regex::AnySymbol>(node))
            {
                // Σ denotes every one-symbol word over the current alphabet.
                const Fragment result = empty_fragment(nfa, false);
                for (Symbol symbol : alphabet)
                {
                    nfa.add_transition(result.start, symbol, result.accept);
                }
                return result;
            }

            if (const auto* star = std::get_if<std::shared_ptr<const regex::KleeneStar>>(&node))
            {
                if (!*star)
                    throw std::invalid_argument("Invalid Kleene-star node");
                const Fragment inner = build((*star)->expression, nfa, alphabet);
                const Fragment result = empty_fragment(nfa, true);
                nfa.add_epsilon_transition(result.start, inner.start);
                nfa.add_epsilon_transition(inner.accept, inner.start);
                nfa.add_epsilon_transition(inner.accept, result.accept);
                return result;
            }

            if (const auto* plus = std::get_if<std::shared_ptr<const regex::Plus>>(&node))
            {
                if (!*plus)
                    throw std::invalid_argument("Invalid plus node");
                const Fragment inner = build((*plus)->expression, nfa, alphabet);
                const Fragment result = empty_fragment(nfa, false);
                nfa.add_epsilon_transition(result.start, inner.start);
                nfa.add_epsilon_transition(inner.accept, inner.start);
                nfa.add_epsilon_transition(inner.accept, result.accept);
                return result;
            }

            if (const auto* concatenation =
                    std::get_if<std::shared_ptr<const regex::Concatenation>>(&node))
            {
                if (!*concatenation)
                    throw std::invalid_argument("Invalid concatenation node");
                if ((*concatenation)->parts.empty())
                    return empty_fragment(nfa, true);

                Fragment result = build((*concatenation)->parts.front(), nfa, alphabet);
                for (std::size_t index = 1; index < (*concatenation)->parts.size(); ++index)
                {
                    const Fragment next = build((*concatenation)->parts[index], nfa, alphabet);
                    nfa.add_epsilon_transition(result.accept, next.start);
                    result.accept = next.accept;
                }
                return result;
            }

            if (const auto* alternation =
                    std::get_if<std::shared_ptr<const regex::Alternation>>(&node))
            {
                if (!*alternation)
                    throw std::invalid_argument("Invalid alternation node");
                const Fragment result = empty_fragment(nfa, false);
                for (const regex::Expression& alternative : (*alternation)->alternatives)
                {
                    const Fragment branch = build(alternative, nfa, alphabet);
                    nfa.add_epsilon_transition(result.start, branch.start);
                    nfa.add_epsilon_transition(branch.accept, result.accept);
                }
                return result;
            }

            if (const auto* intersection =
                        std::get_if<std::shared_ptr<const regex::Intersection>>(&node))
            {
                if (!*intersection)
                    throw std::invalid_argument("Invalid intersection node");
                if ((*intersection)->operands.empty())
                    return empty_fragment(nfa, true);

                std::vector<mata::nfa::Nfa> operands;
                operands.reserve((*intersection)->operands.size());

                for (const regex::Expression& operand : (*intersection)->operands)
                {
                    operands.push_back(
                        detail::minimize(detail::to_mata(to_nfa(operand, alphabet))).trim()
                    );
                }

                return append(detail::from_mata(intersect_all(std::move(operands))), nfa);
            }

            if (const auto* complement =
                         std::get_if<std::shared_ptr<const regex::Complement>>(&node))
            {
                if (!*complement)
                    throw std::invalid_argument("Invalid complement node");
                // Complement requires a complete deterministic automaton over the active alphabet.
                const mata::nfa::Nfa inner =
                    detail::minimize(detail::to_mata(to_nfa((*complement)->expression, alphabet)));

                mata::nfa::Nfa complemented =
                    mata::nfa::complement(inner, detail::to_mata_alphabet(alphabet)).trim();

                return append(detail::from_mata(complemented), nfa);
            }

            if (const auto* power = std::get_if<std::shared_ptr<const regex::Power>>(&node))
            {
                if (!*power)
                    throw std::invalid_argument("Invalid power node");
                if ((*power)->exponent == 0)
                    return empty_fragment(nfa, true);

                Fragment result = build((*power)->expression, nfa, alphabet);
                for (unsigned int exponent = 1; exponent < (*power)->exponent; ++exponent)
                {
                    const Fragment next = build((*power)->expression, nfa, alphabet);
                    nfa.add_epsilon_transition(result.accept, next.start);
                    result.accept = next.accept;
                }
                return result;
            }

            throw std::invalid_argument("Unsupported regular-expression node");
        }
    }

    Nfa to_nfa(const regex::Expression& expression)
    {
        return to_nfa(expression, alphabet_of(expression));
    }

    Nfa to_nfa(const regex::Expression& expression, const Alphabet& alphabet)
    {
        Nfa result;
        result.alphabet = alphabet;
        const regex::Expression normalized = regex::simplification::normalize(expression);
        const Fragment fragment = build(normalized, result, alphabet);
        result.start = fragment.start;
        result.finals.insert(fragment.accept);
        return result;
    }
}
