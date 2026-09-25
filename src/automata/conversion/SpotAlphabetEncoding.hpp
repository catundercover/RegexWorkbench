// Declares the finite-alphabet encoding used by project-owned Spot automata.
#pragma once

#include "automata/model/Nfa.hpp"

#include <spot/twa/twagraph.hh>
#include <vector>

namespace automata::conversion::spot_encoding
{
    // Registers the Boolean variables needed to represent every alphabet symbol.
    void register_alphabet(const spot::twa_graph_ptr& automaton, const Alphabet& alphabet);

    // Returns the complete Boolean valuation assigned to one alphabet symbol.
    [[nodiscard]] bdd
    symbol_condition(const spot::twa_graph_ptr& automaton, const Alphabet& alphabet, Symbol symbol);

    // Decodes all alphabet symbols whose valuation satisfies a Spot condition.
    [[nodiscard]] std::vector<Symbol> satisfying_symbols(
        const spot::twa_graph_ptr& automaton, const Alphabet& alphabet, const bdd& condition
    );
}
