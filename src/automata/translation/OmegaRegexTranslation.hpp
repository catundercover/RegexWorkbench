// Declares high-level omega regular-expression translation pipelines.
#pragma once

#include "automata/model/BuchiAutomaton.hpp"
#include "regex/model/OmegaExpression.hpp"

#include <optional>
#include <spot/twa/twagraph.hh>

namespace automata::translation
{
    // Produces a Spot Büchi automaton over the merged alphabet.
    [[nodiscard]] spot::twa_graph_ptr
    buchi_twa(const regex::omega::Expression& expression, const Alphabet& extra_alphabet = {});

    // Produces a language-equivalent Spot automaton optimized for internal language operations. The acceptance condition is not guaranteed to be Büchi.
    [[nodiscard]] spot::twa_graph_ptr optimized_twa(const regex::omega::Expression& expression,
        const Alphabet& extra_alphabet = {});

    // Produces a UI-friendly Büchi automaton over the merged alphabet.
    [[nodiscard]] BuchiAutomaton buchi_automaton(
        const regex::omega::Expression& expression, const Alphabet& extra_alphabet = {}
    );

    // Returns a minimal state-based DBA, or nullopt iff the language has no DBA.
    // Missing transitions reject; rejecting sink states are omitted. Exact SAT
    // minimization can be expensive and should run in an isolated worker.
    [[nodiscard]] std::optional<BuchiAutomaton>
    minimal_dba(const regex::omega::Expression& expression, const Alphabet& extra_alphabet = {});
}
