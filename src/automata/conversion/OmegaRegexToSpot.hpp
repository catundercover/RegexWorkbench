// Declares conversion from omega regular expressions to Spot automata.
#pragma once

#include "automata/model/Nfa.hpp"
#include "regex/model/OmegaExpression.hpp"

#include <spot/twa/twagraph.hh>

namespace automata::conversion
{
    // Builds a raw Spot automaton without final postprocessing.
    [[nodiscard]] spot::twa_graph_ptr
    to_spot_twa_raw(const regex::omega::Expression& expression, const Alphabet& alphabet);

    // Builds a raw Spot automaton using a caller-owned shared BDD dictionary.
    //
    // Automata passed to Spot product operations must share the same dictionary.
    [[nodiscard]] spot::twa_graph_ptr to_spot_twa_raw(
        const regex::omega::Expression& expression,
        const Alphabet& alphabet,
        const spot::bdd_dict_ptr& dictionary
    );

    // Builds and postprocesses a Spot automaton over the expression alphabet.
    [[nodiscard]] spot::twa_graph_ptr to_spot_twa(const regex::omega::Expression& expression);

    // Builds and postprocesses a Spot automaton over the supplied alphabet.
    [[nodiscard]] spot::twa_graph_ptr
    to_spot_twa(const regex::omega::Expression& expression, const Alphabet& alphabet);

    // Produces a language-equivalent Spot automaton optimized for internal language operations.
    // The result is not guaranteed to be a Büchi automaton.
    [[nodiscard]] spot::twa_graph_ptr
    to_optimized_spot_twa(const regex::omega::Expression& expression, const Alphabet& alphabet);
}
