// Implements conversion from Spot automata to project state-based Büchi display models.
#include "automata/conversion/SpotOmegaAdapter.hpp"

#include "automata/conversion/SpotAlphabetEncoding.hpp"

#include <spot/misc/optionmap.hh>
#include <spot/twaalgos/postproc.hh>
#include <stdexcept>

namespace automata::conversion::spot_adapter
{
    namespace
    {
        spot::twa_graph_ptr request_state_based_buchi(const spot::twa_graph_ptr& automaton)
        {
            if (!automaton)
            {
                throw std::invalid_argument("Cannot convert a null Spot automaton");
            }

            // Display-ready automata (including SAT-minimized DBAs) must not
            // pass through another, potentially nondeterministic pipeline.
            if (automaton->acc().is_buchi() && automaton->prop_state_acc().is_true())
            {
                return automaton;
            }

            spot::option_map options;
            options.set("ba-simul", 1);
            spot::postprocessor postprocessor(&options);
            postprocessor.set_type(spot::postprocessor::BA);
            postprocessor.set_pref(spot::postprocessor::Small | spot::postprocessor::SBAcc);
            postprocessor.set_level(spot::postprocessor::Medium);
            return postprocessor.run(automaton);
        }
    }

    BuchiAutomaton
    to_state_based_buchi_automaton(const spot::twa_graph_ptr& automaton, const Alphabet& alphabet)
    {
        const spot::twa_graph_ptr state_based = request_state_based_buchi(automaton);

        BuchiAutomaton result;
        result.start = state_based->get_init_state_number();
        result.transitions.resize(state_based->num_states());
        result.alphabet = alphabet;

        for (unsigned state = 0; state < state_based->num_states(); ++state)
        {
            if (state_based->state_is_accepting(state))
            {
                result.finals.insert(static_cast<StateId>(state));
            }

            for (const auto& edge : state_based->out(state))
            {
                const std::vector<Symbol> symbols =
                    spot_encoding::satisfying_symbols(state_based, alphabet, edge.cond);
                if (symbols.empty())
                {
                    continue;
                }

                result.transitions[state].push_back(
                    BuchiTransition{
                        Alphabet(symbols.begin(), symbols.end()), static_cast<StateId>(edge.dst)
                    }
                );
            }
        }

        return result;
    }
}
