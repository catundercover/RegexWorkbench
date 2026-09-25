// Declares the interactive automaton graph canvas.
#pragma once

#include "ui/graph/GraphCanvasModel.hpp"

struct ImVec2;

namespace graph
{
    struct Layout;
}

namespace ui::graph
{
    // Renders an interactive Graphviz layout inside an ImGui child canvas.
    void render_automaton_graph(
        const ::graph::Layout& layout,
        GraphCanvasState& state,
        const ImVec2& size,
        const char* child_id
    );
}
