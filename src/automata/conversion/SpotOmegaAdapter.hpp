// Declares conversion helpers between Spot automata and project display models.
#pragma once

#include "automata/model/BuchiAutomaton.hpp"

#include <spot/twa/twagraph.hh>

namespace automata::conversion::spot_adapter
{
    // Requests a state-based Büchi automaton from Spot and converts it to the
    // project-owned display model.
    [[nodiscard]] BuchiAutomaton
    to_state_based_buchi_automaton(const spot::twa_graph_ptr& automaton, const Alphabet& alphabet);
}