// Declares state-elimination conversion from Spot automata to omega regexes.
#pragma once

#include "automata/model/Nfa.hpp"
#include "regex/model/OmegaExpression.hpp"

#include <spot/twa/twagraph.hh>

namespace automata::conversion
{
    // Converts a Spot automaton into an equivalent omega expression over `alphabet`.
    [[nodiscard]] regex::omega::Expression
    to_omega_regex_ast(const spot::twa_graph_ptr& automaton, const Alphabet& alphabet);
}
