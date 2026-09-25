// Declares high-level regular-expression translation pipelines.
#pragma once

#include "automata/model/Nfa.hpp"
#include "regex/model/Expression.hpp"

namespace automata::translation
{
    // Produces a deterministic minimal automaton over the merged alphabet.
    [[nodiscard]] Nfa
    minimal_dfa(const regex::Expression& expression, const Alphabet& extra_alphabet = {});

    // Produces a reduced NFA without epsilon transitions.
    [[nodiscard]] Nfa
    epsilon_free_nfa(const regex::Expression& expression, const Alphabet& extra_alphabet = {});
}
