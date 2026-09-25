// Declares terminal-alphabet extraction from omega expression trees.
#pragma once

#include "automata/model/Nfa.hpp"
#include "regex/model/OmegaExpression.hpp"

namespace automata::conversion
{
    // Collects all explicit terminals in finite operands of an omega expression.
    [[nodiscard]] Alphabet alphabet_of(const regex::omega::Expression& expression);
    // Collects explicit terminals and merges a caller-supplied alphabet.
    [[nodiscard]] Alphabet
    alphabet_of(const regex::omega::Expression& expression, const Alphabet& extra_alphabet);
}
