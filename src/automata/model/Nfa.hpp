// Defines the project-owned nondeterministic finite automaton model.
#pragma once

#include <cstddef>
#include <optional>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace automata
{
    // Stable index of a state in an NFA transition table.
    using StateId = std::size_t;
    // Terminal symbol consumed by an automaton transition.
    using Symbol = char;
    // Set of terminal symbols over which an automaton is interpreted.
    using Alphabet = std::unordered_set<Symbol>;

    // One labeled or epsilon transition to a target state.
    struct Transition
    {
        std::optional<Symbol> symbol;
        StateId target;
    };

    // Small, project-owned representation used at every public automata boundary.
    struct Nfa
    {
        StateId start = 0;
        std::unordered_set<StateId> finals;
        std::vector<std::vector<Transition>> transitions;
        Alphabet alphabet;

        // Appends an empty state and returns its identifier.
        [[nodiscard]] StateId add_state();
        // Adds an epsilon transition after validating both state identifiers.
        void add_epsilon_transition(StateId from, StateId to);
        /// Adds a symbol transition and records the symbol in the alphabet.
        void add_transition(StateId from, Symbol symbol, StateId to);

        // Checks state references, the start state, and transition labels.
        [[nodiscard]] bool is_valid() const noexcept;
        // Returns whether the NFA accepts the complete input word.
        [[nodiscard]] bool accepts(std::string_view word) const;
    };
}
