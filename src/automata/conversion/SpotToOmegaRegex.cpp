// Implements state-elimination conversion from Spot automata to omega regexes.
#include "automata/conversion/SpotToOmegaRegex.hpp"

#include "automata/conversion/SpotOmegaAdapter.hpp"
#include "regex/simplification/OmegaRegexSimplifier.hpp"
#include "regex/simplification/RegexSimplifier.hpp"
#include "regex/inspection/ExpressionInspection.hpp"

#include <optional>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

namespace automata::conversion
{
    namespace
    {
        regex::Expression epsilon_finite()
        {
            return regex::Epsilon{};
        }

        bool is_empty(const regex::Expression& expression)
        {
            return std::holds_alternative<regex::EmptySet>(expression);
        }

        regex::Expression unite(regex::Expression left, regex::Expression right)
        {
            if (is_empty(left))
            {
                return right;
            }
            if (is_empty(right))
            {
                return left;
            }
            return regex::simplification::normalize(
                regex::make_alternation({std::move(left), std::move(right)})
            );
        }

        regex::Expression concatenate(regex::Expression left, regex::Expression right)
        {
            if (is_empty(left) || is_empty(right))
            {
                return regex::EmptySet{};
            }
            return regex::simplification::normalize(
                regex::make_concatenation({std::move(left), std::move(right)})
            );
        }

        regex::Expression star(regex::Expression expression)
        {
            if (is_empty(expression) || std::holds_alternative<regex::Epsilon>(expression))
            {
                return regex::Epsilon{};
            }
            return regex::simplification::normalize(regex::make_star(std::move(expression)));
        }

        regex::Expression transition_symbols_to_finite_regex(
            const Alphabet& symbols, const Alphabet& complete_alphabet
        )
        {
            if (symbols == complete_alphabet)
            {
                return regex::AnySymbol{};
            }

            std::vector<Symbol> ordered(symbols.begin(), symbols.end());
            std::ranges::sort(ordered);

            std::vector<regex::Expression> alternatives;
            alternatives.reserve(ordered.size());
            for (const Symbol symbol : ordered)
            {
                alternatives.push_back(regex::make_terminal(symbol));
            }
            return regex::make_alternation(std::move(alternatives));
        }

        bool transition_covers_alphabet(
               const BuchiTransition& transition, const Alphabet& complete_alphabet
           )
        {
            return transition.symbols == complete_alphabet;
        }

        bool is_canonical_universal_buchi(const BuchiAutomaton& automaton)
        {
            if (automaton.transitions.size() != 1 || automaton.start != 0 ||
                !automaton.finals.contains(0))
            {
                return false;
            }

            return std::any_of(
                automaton.transitions[0].begin(),
                automaton.transitions[0].end(),
                [&automaton](const BuchiTransition& transition)
                {
                    return transition.target == 0 &&
                           transition_covers_alphabet(transition, automaton.alphabet);
                }
            );
        }

        using PathMatrix = std::vector<std::vector<regex::Expression>>;

        PathMatrix transition_matrix(const BuchiAutomaton& automaton)
        {
            const std::size_t state_count = automaton.transitions.size();

            PathMatrix matrix(
                state_count, std::vector<regex::Expression>(state_count, regex::EmptySet{})
            );

            for (std::size_t state = 0; state < state_count; ++state)
            {
                for (const BuchiTransition& transition : automaton.transitions[state])
                {
                    matrix[state][transition.target] = unite(
                        std::move(matrix[state][transition.target]),
                        transition_symbols_to_finite_regex(transition.symbols, automaton.alphabet)
                    );
                }
            }
            return matrix;
        }

        void eliminate_state(PathMatrix& matrix, const StateId eliminated)
        {
            const regex::Expression loop = star(matrix[eliminated][eliminated]);

            for (StateId source = 0; source < matrix.size(); ++source)
            {
                if (source == eliminated || is_empty(matrix[source][eliminated]))
                {
                    continue;
                }

                for (StateId target = 0; target < matrix.size(); ++target)
                {
                    if (target == eliminated || is_empty(matrix[eliminated][target]))
                    {
                        continue;
                    }

                    regex::Expression path = concatenate(
                        concatenate(matrix[source][eliminated], loop), matrix[eliminated][target]
                    );

                    matrix[source][target] =
                        unite(std::move(matrix[source][target]), std::move(path));
                }
            }

            for (StateId state = 0; state < matrix.size(); ++state)
            {
                matrix[state][eliminated] = regex::EmptySet{};
                matrix[eliminated][state] = regex::EmptySet{};
            }
        }

        std::size_t finite_node_count(const regex::Expression& expression)
        {
            return regex::inspection::node_count(expression);
        }

        std::size_t elimination_cost(
                const PathMatrix& matrix, const StateId eliminated, const std::vector<bool>& remaining
            )
        {
            std::size_t incoming = 0;
            std::size_t outgoing = 0;
            std::size_t size_sum = 1;

            for (StateId state = 0; state < matrix.size(); ++state)
            {
                if (!remaining[state] || state == eliminated)
                {
                    continue;
                }

                if (!is_empty(matrix[state][eliminated]))
                {
                    ++incoming;
                    size_sum += finite_node_count(matrix[state][eliminated]);
                }

                if (!is_empty(matrix[eliminated][state]))
                {
                    ++outgoing;
                    size_sum += finite_node_count(matrix[eliminated][state]);
                }
            }

            if (!is_empty(matrix[eliminated][eliminated]))
            {
                size_sum += finite_node_count(matrix[eliminated][eliminated]);
            }

            if (incoming == 0 || outgoing == 0)
            {
                return 0;
            }

            return incoming * outgoing * size_sum;
        }

        void eliminate_states_heuristically(
            PathMatrix& matrix, const std::vector<bool>& protected_states
        )
        {
            std::vector<bool> remaining(matrix.size(), true);

            while (true)
            {
                std::optional<StateId> best_state;
                std::size_t best_cost = 0;

                for (StateId state = 0; state < matrix.size(); ++state)
                {
                    if (!remaining[state] || protected_states[state])
                    {
                        continue;
                    }

                    const std::size_t cost = elimination_cost(matrix, state, remaining);

                    if (!best_state.has_value() || cost < best_cost)
                    {
                        best_state = state;
                        best_cost = cost;
                    }
                }

                if (!best_state.has_value())
                {
                    break;
                }

                eliminate_state(matrix, *best_state);
                remaining[*best_state] = false;
            }
        }

        regex::Expression
             paths_to_accepting(const BuchiAutomaton& automaton, const StateId accepting)
        {
            if (automaton.start == accepting)
            {
                return epsilon_finite();
            }

            PathMatrix matrix = transition_matrix(automaton);

            std::vector<bool> protected_states(matrix.size(), false);
            protected_states[automaton.start] = true;
            protected_states[accepting] = true;

            eliminate_states_heuristically(matrix, protected_states);

            return concatenate(
                star(std::move(matrix[automaton.start][automaton.start])),
                std::move(matrix[automaton.start][accepting])
            );
        }

        regex::Expression
        nonempty_cycles_at(const BuchiAutomaton& automaton, const StateId accepting)
        {
            PathMatrix matrix = transition_matrix(automaton);

            std::vector<bool> protected_states(matrix.size(), false);
            protected_states[accepting] = true;

            eliminate_states_heuristically(matrix, protected_states);

            return std::move(matrix[accepting][accepting]);
        }
    }

    regex::omega::Expression
    to_omega_regex_ast(const spot::twa_graph_ptr& automaton, const Alphabet& alphabet)
    {
        if (!automaton)
        {
            throw std::invalid_argument("Cannot convert a null Spot automaton to omega regex");
        }

        const BuchiAutomaton buchi =
            spot_adapter::to_state_based_buchi_automaton(automaton, alphabet);

        if (!buchi.is_valid())
        {
            throw std::runtime_error("Spot produced an invalid Büchi automaton");
        }

        if (is_canonical_universal_buchi(buchi))
        {
            return regex::omega::UniversalSet{};
        }

        std::vector<regex::omega::Expression> alternatives;

        for (StateId accepting : buchi.finals)
        {
            regex::Expression prefix = paths_to_accepting(buchi, accepting);
            regex::Expression cycle = nonempty_cycles_at(buchi, accepting);

            if (is_empty(prefix) || is_empty(cycle))
            {
                continue;
            }

            alternatives.push_back(
                regex::omega::make_concatenation(
                    std::move(prefix), regex::omega::make_omega_power(std::move(cycle))
                )
            );
        }

        return regex::simplification::omega::normalize(
            regex::omega::make_alternation(std::move(alternatives))
        );
    }
}
