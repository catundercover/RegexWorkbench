// Declares Graphviz DOT export for Büchi automata.
#pragma once

#include "automata/model/BuchiAutomaton.hpp"

#include <string>

namespace automata::dot
{
    // Serializes a valid Büchi automaton to deterministic Graphviz DOT.
    [[nodiscard]] std::string to_dot(const BuchiAutomaton& automaton);
}