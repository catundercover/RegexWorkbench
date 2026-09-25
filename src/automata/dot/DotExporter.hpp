// Declares deterministic Graphviz DOT export for automata.
#pragma once

#include "automata/model/Nfa.hpp"

#include <string>

namespace automata::dot
{
    // Serializes a valid automaton to deterministic Graphviz DOT.
    [[nodiscard]] std::string to_dot(const Nfa& nfa);
}
