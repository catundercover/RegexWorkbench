// Defines a small project-owned state-based Büchi automaton view model for rendering.
#pragma once

#include "automata/model/Nfa.hpp"

#include <unordered_set>
#include <vector>

namespace automata
{
    // One Büchi transition over a non-empty set of character symbols.
    struct BuchiTransition
    {
        Alphabet symbols;
        StateId target = 0;
    };

    // UI-friendly state-based Büchi automaton representation (NBA or DBA).
    //
    // Semantics:
    //     an infinite run is accepting iff it visits states from `finals`
    //     infinitely often.
    //
    // Spot remains the authoritative automaton representation for algorithms.
    // This type exists so the existing DOT/Graphviz/UI path can render a
    // classical state-based NBA or DBA.
    struct BuchiAutomaton
    {
        StateId start = 0;
        std::unordered_set<StateId> finals;
        std::vector<std::vector<BuchiTransition>> transitions;
        Alphabet alphabet;

        [[nodiscard]] bool is_valid() const noexcept
        {
            if (transitions.empty() || start >= transitions.size())
            {
                return false;
            }

            for (StateId final_state : finals)
            {
                if (final_state >= transitions.size())
                {
                    return false;
                }
            }

            for (const auto& outgoing : transitions)
            {
                for (const BuchiTransition& transition : outgoing)
                {
                    if (transition.target >= transitions.size() || transition.symbols.empty())
                    {
                        return false;
                    }

                    for (const Symbol symbol : transition.symbols)
                    {
                        if (!alphabet.contains(symbol))
                        {
                            return false;
                        }
                    }
                }
            }

            return true;
        }
    };
}
