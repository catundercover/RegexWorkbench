// Declares formal, copyable text formatting for rendered automata.
#pragma once

#include <string>

namespace graph
{
    struct Layout;
}

namespace ui::graph
{
    // Formats a graph layout as the formal tuple
    [[nodiscard]] std::string format_automaton_text(const ::graph::Layout& layout);
}
