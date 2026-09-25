// Implements terminal-alphabet extraction from expression trees.
#include "automata/conversion/RegexAlphabet.hpp"

#include "regex/inspection/ExpressionInspection.hpp"

namespace automata::conversion
{
    Alphabet alphabet_of(const regex::Expression& expression)
    {
        const std::set<char> terminals = regex::inspection::terminals(expression);
        return Alphabet(terminals.begin(), terminals.end());
    }

    Alphabet alphabet_of(const regex::Expression& expression, const Alphabet& extra_alphabet)
    {
        Alphabet alphabet = alphabet_of(expression);
        alphabet.insert(extra_alphabet.begin(), extra_alphabet.end());
        return alphabet;
    }
}
