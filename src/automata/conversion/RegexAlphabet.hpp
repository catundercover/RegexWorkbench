// Declares alphabet extraction for regular-expression trees.
#pragma once

#include "automata/model/Nfa.hpp"
#include "regex/model/Expression.hpp"

namespace automata::conversion
{
    // Returns every terminal occurring in the expression.
    [[nodiscard]] Alphabet alphabet_of(const regex::Expression& expression);

    // Returns the expression alphabet extended with explicitly supplied symbols.
    [[nodiscard]] Alphabet
    alphabet_of(const regex::Expression& expression, const Alphabet& extra_alphabet);
}
