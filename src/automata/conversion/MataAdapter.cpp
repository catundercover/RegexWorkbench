// Implements checked conversion between project and MATA automata.
#include "automata/conversion/MataAdapter.hpp"

#include <stdexcept>

namespace automata::conversion::detail
{
    namespace
    {
        // Converts a checked project state index to MATA's state type.
        mata::nfa::State to_mata_state(StateId state)
        {
            return static_cast<mata::nfa::State>(state);
        }

        // Preserves the unsigned byte value while converting a terminal to MATA.
        mata::Symbol to_mata_symbol(Symbol symbol)
        {
            return static_cast<mata::Symbol>(static_cast<unsigned char>(symbol));
        }
    }

    mata::nfa::Nfa to_mata(const Nfa& nfa)
    {
        if (!nfa.is_valid())
        {
            throw std::invalid_argument("Cannot convert an invalid NFA");
        }

        mata::nfa::Nfa result;
        for (StateId state = 0; state < nfa.transitions.size(); ++state)
        {
            result.add_state();
        }

        result.initial.insert(to_mata_state(nfa.start));
        for (StateId final_state : nfa.finals)
        {
            result.final.insert(to_mata_state(final_state));
        }

        for (StateId source = 0; source < nfa.transitions.size(); ++source)
        {
            for (const Transition& transition : nfa.transitions[source])
            {
                const mata::Symbol symbol =
                    transition.symbol ? to_mata_symbol(*transition.symbol) : mata::nfa::EPSILON;
                result.delta.add(to_mata_state(source), symbol, to_mata_state(transition.target));
            }
        }

        return result;
    }

    Nfa from_mata(const mata::nfa::Nfa& source)
    {
        Nfa result;
        for (std::size_t state = 0; state < source.num_of_states(); ++state)
        {
            (void)result.add_state();
        }

        if (source.initial.size() == 1)
        {
            result.start = static_cast<StateId>(*source.initial.begin());
        }
        else
        {
            const StateId synthetic_start = result.add_state();
            result.start = synthetic_start;
            for (mata::nfa::State initial : source.initial)
            {
                result.add_epsilon_transition(synthetic_start, static_cast<StateId>(initial));
            }
        }

        for (mata::nfa::State final_state : source.final)
        {
            result.finals.insert(static_cast<StateId>(final_state));
        }

        for (mata::nfa::State source_state = 0; source_state < source.num_of_states();
             ++source_state)
        {
            for (const auto& transition : source.delta[source_state])
            {
                for (mata::nfa::State target : transition.targets)
                {
                    if (transition.symbol == mata::nfa::EPSILON)
                    {
                        result.add_epsilon_transition(
                            static_cast<StateId>(source_state), static_cast<StateId>(target)
                        );
                    }
                    else
                    {
                        result.add_transition(
                            static_cast<StateId>(source_state),
                            static_cast<Symbol>(static_cast<unsigned char>(transition.symbol)),
                            static_cast<StateId>(target)
                        );
                    }
                }
            }
        }

        return result;
    }

    mata::nfa::Nfa minimize(const mata::nfa::Nfa& nfa)
    {
        return mata::nfa::minimize(mata::nfa::determinize(mata::nfa::remove_epsilon(nfa)));
    }

    mata::utils::OrdVector<mata::Symbol> to_mata_alphabet(const Alphabet& alphabet)
    {
        mata::utils::OrdVector<mata::Symbol> result;
        for (Symbol symbol : alphabet)
        {
            result.insert(to_mata_symbol(symbol));
        }
        return result;
    }

    std::optional<std::string> shortest_word(const mata::nfa::Nfa& nfa)
    {
        mata::nfa::Run accepting_run;
        if (nfa.is_lang_empty(&accepting_run))
        {
            return std::nullopt;
        }

        std::string result;
        result.reserve(accepting_run.word.size());
        for (mata::Symbol symbol : accepting_run.word)
        {
            if (symbol != mata::nfa::EPSILON)
            {
                result.push_back(static_cast<char>(static_cast<unsigned char>(symbol)));
            }
        }
        return result;
    }
}
