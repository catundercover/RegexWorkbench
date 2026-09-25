// Declares Thompson-style regular-expression to NFA conversion.
#pragma once

#include "automata/model/Nfa.hpp"
#include "regex/model/Expression.hpp"

namespace automata::conversion
{
    // Builds a Thompson NFA over the expression's own alphabet.
    [[nodiscard]] Nfa to_nfa(const regex::Expression& expression);

    // Builds a Thompson NFA using the supplied alphabet for complement and any-symbol nodes.
    [[nodiscard]] Nfa to_nfa(const regex::Expression& expression, const Alphabet& alphabet);
}
