// Implements NFA construction, validation, and word acceptance.
#include "automata/model/Nfa.hpp"

#include <stdexcept>
#include <utility>

namespace automata
{
    namespace
    {
        // Rejects a state identifier outside an automaton's transition table.
        void require_state(const Nfa& nfa, StateId state)
        {
            if (state >= nfa.transitions.size())
            {
                throw std::out_of_range("NFA state does not exist");
            }
        }

        // Expands a state set through every reachable epsilon transition.
        std::unordered_set<StateId>
        epsilon_closure(const Nfa& nfa, std::unordered_set<StateId> states)
        {
            std::vector<StateId> pending(states.begin(), states.end());

            while (!pending.empty())
            {
                const StateId state = pending.back();
                pending.pop_back();

                for (const Transition& transition : nfa.transitions[state])
                {
                    if (!transition.symbol.has_value() && states.insert(transition.target).second)
                    {
                        pending.push_back(transition.target);
                    }
                }
            }

            return states;
        }
    }

    StateId Nfa::add_state()
    {
        transitions.emplace_back();
        return transitions.size() - 1;
    }

    void Nfa::add_epsilon_transition(StateId from, StateId to)
    {
        require_state(*this, from);
        require_state(*this, to);
        transitions[from].push_back(Transition{std::nullopt, to});
    }

    void Nfa::add_transition(StateId from, Symbol symbol, StateId to)
    {
        require_state(*this, from);
        require_state(*this, to);
        transitions[from].push_back(Transition{symbol, to});
        alphabet.insert(symbol);
    }

    bool Nfa::is_valid() const noexcept
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
            for (const Transition& transition : outgoing)
            {
                if (transition.target >= transitions.size())
                {
                    return false;
                }
            }
        }

        return true;
    }

    bool Nfa::accepts(std::string_view word) const
    {
        if (!is_valid())
        {
            throw std::invalid_argument("Cannot evaluate an invalid NFA");
        }

        std::unordered_set<StateId> current = epsilon_closure(*this, {start});

        for (Symbol symbol : word)
        {
            std::unordered_set<StateId> next;

            for (StateId state : current)
            {
                for (const Transition& transition : transitions[state])
                {
                    if (transition.symbol == symbol)
                    {
                        next.insert(transition.target);
                    }
                }
            }

            current = epsilon_closure(*this, std::move(next));
        }

        for (StateId state : current)
        {
            if (finals.contains(state))
            {
                return true;
            }
        }

        return false;
    }
}
