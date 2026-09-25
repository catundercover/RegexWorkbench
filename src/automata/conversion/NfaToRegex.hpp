// Declares conversion from finite automata to regular expressions.
#pragma once

#include "automata/model/Nfa.hpp"
#include "regex/model/Expression.hpp"

#include <string>

namespace automata::conversion
{
    // Converts an automaton to a regular-expression AST by state elimination.
    [[nodiscard]] regex::Expression to_regex_ast(const Nfa& nfa);

    // Converts an automaton to displayable regular-expression text.
    [[nodiscard]] std::string to_regex(const Nfa& nfa);
}
