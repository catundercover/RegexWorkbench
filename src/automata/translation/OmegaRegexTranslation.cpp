// Implements high-level omega regular-expression translation pipelines.
#include "automata/translation/OmegaRegexTranslation.hpp"

#include "automata/conversion/OmegaRegexAlphabet.hpp"
#include "automata/conversion/OmegaRegexToSpot.hpp"
#include "automata/conversion/SpotAlphabetEncoding.hpp"
#include "automata/conversion/SpotOmegaAdapter.hpp"

#include <limits>
#include <spot/twaalgos/complete.hh>
#include <spot/twaalgos/degen.hh>
#include <spot/twaalgos/dtbasat.hh>
#include <spot/twaalgos/isdet.hh>
#include <spot/twaalgos/minimize.hh>
#include <spot/twaalgos/postproc.hh>
#include <spot/twaalgos/remfin.hh>
#include <spot/twaalgos/sbacc.hh>
#include <spot/twaalgos/sccfilter.hh>
#include <spot/twaalgos/strength.hh>
#include <spot/twaalgos/totgba.hh>
#include <stdexcept>
#include <utility>
#include <vector>

namespace automata::translation
{
    namespace
    {
        spot::twa_graph_ptr deterministic_buchi(spot::twa_graph_ptr automaton)
        {
            // Preserve compact transition acceptance throughout determinization.
            // A failed Buchi+Deterministic preference would not prove that the
            // language has no DBA; deterministic parity gives an exact test.
            if (!spot::is_deterministic(automaton) || !automaton->acc().is_generalized_buchi())
            {
                spot::postprocessor determinizer;
                determinizer.set_type(spot::postprocessor::Parity);
                determinizer.set_pref(spot::postprocessor::Deterministic);
                determinizer.set_level(spot::postprocessor::Medium);
                automaton = determinizer.run(automaton);
            }

            if (!spot::is_deterministic(automaton))
            {
                throw std::runtime_error("Spot could not finish determinization.");
            }

            if (automaton->acc().is_generalized_buchi())
            {
                return spot::degeneralize_tba(automaton);
            }

            // Expanding parity acceptance to Rabin changes only marks, not
            // states. The realizability test is conclusive on deterministic
            // Rabin automata, and its successful conversion preserves structure.
            const auto rabin = spot::to_generalized_rabin(automaton);
            std::vector<spot::acc_cond::rs_pair> pairs;
            if (!rabin->acc().is_rabin_like(pairs))
            {
                throw std::runtime_error("Spot did not produce Rabin acceptance for the DBA test.");
            }
            return spot::rabin_to_buchi_if_realizable(rabin);
        }

        void
        restore_letter_conditions(const spot::twa_graph_ptr& automaton, const Alphabet& alphabet)
        {
            // For non-power-of-two alphabets, several Boolean valuations encode
            // the last letter. SAT may give these different successors. Choose
            // one fixed representative per letter and extend its behavior to
            // all aliases, preserving language and character-level determinism.
            conversion::spot_encoding::register_alphabet(automaton, alphabet);
            std::vector<std::pair<bdd, bdd>> letters;
            for (const Symbol symbol : alphabet)
            {
                const bdd condition =
                    conversion::spot_encoding::symbol_condition(automaton, alphabet, symbol);
                letters.emplace_back(
                    condition, bdd_satoneset(condition, automaton->ap_vars(), bddfalse)
                );
            }
            for (auto& edge : automaton->edges())
            {
                bdd condition = bddfalse;
                for (const auto& [letter, representative] : letters)
                {
                    if ((edge.cond & representative) != bddfalse)
                    {
                        condition |= letter;
                    }
                }
                edge.cond = condition;
            }
            automaton->merge_edges();
            automaton->purge_unreachable_states();
        }
    }

    spot::twa_graph_ptr
    buchi_twa(const regex::omega::Expression& expression, const Alphabet& extra_alphabet)
    {
        const Alphabet alphabet = conversion::alphabet_of(expression, extra_alphabet);
        return conversion::to_spot_twa(expression, alphabet);
    }

    spot::twa_graph_ptr
    optimized_twa(const regex::omega::Expression& expression, const Alphabet& extra_alphabet)
    {
        const Alphabet alphabet = conversion::alphabet_of(expression, extra_alphabet);
        return conversion::to_optimized_spot_twa(expression, alphabet);
    }

    BuchiAutomaton
        buchi_automaton(const regex::omega::Expression& expression, const Alphabet& extra_alphabet)
    {
        const Alphabet alphabet = conversion::alphabet_of(expression, extra_alphabet);
        const spot::twa_graph_ptr automaton = conversion::to_spot_twa(expression, alphabet);
        return conversion::spot_adapter::to_state_based_buchi_automaton(automaton, alphabet);
    }

    std::optional<BuchiAutomaton>
    minimal_dba(const regex::omega::Expression& expression, const Alphabet& extra_alphabet)
    {
        const Alphabet alphabet = conversion::alphabet_of(expression, extra_alphabet);
        spot::twa_graph_ptr transition_based =
            deterministic_buchi(conversion::to_optimized_spot_twa(expression, alphabet));
        if (!transition_based)
        {
            return std::nullopt;
        }

        spot::twa_graph_ptr state_based;
        if (spot::is_weak_automaton(transition_based))
        {
            // Weak deterministic languages admit inexpensive exact minimization.
            // Avoid SAT entirely for safety and finite-prefix examples.
            state_based =
                spot::sbacc(spot::degeneralize_tba(spot::minimize_wdba(transition_based)));
        }
        else
        {
            // SAT synthesizes complete candidates. Keep its reference transition
            // based, but use a *state-based* upper bound: minimizing a TBA and then
            // splitting its states would not establish a minimal DBA.
            spot::complete_here(transition_based);
            state_based = spot::sbacc(transition_based);
            if (state_based->num_states() > static_cast<unsigned>(std::numeric_limits<int>::max()))
            {
                throw std::overflow_error("DBA state count exceeds Spot's SAT minimization limit.");
            }
            if (state_based->num_states() > 1)
            {
                const auto smaller = spot::dtba_sat_minimize_dichotomy(
                    transition_based, true, false, static_cast<int>(state_based->num_states()) - 1
                );
                if (smaller)
                {
                    state_based = smaller;
                }
            }
        }

        // A minimal complete DBA has at most one empty-language sink. Removing
        // it yields a minimal partial DBA (completion adds at most one state).
        // Spot retains one initial state for the empty language.
        restore_letter_conditions(state_based, alphabet);
        state_based = spot::scc_filter_states(state_based);
        return conversion::spot_adapter::to_state_based_buchi_automaton(state_based, alphabet);
    }
}
