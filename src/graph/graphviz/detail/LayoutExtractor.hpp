// Declares extraction of UI geometry from a laid-out Graphviz graph.
#pragma once

#include "graph/graphviz/GraphvizLayout.hpp"

struct Agraph_s;

namespace graph::graphviz::detail
{
    // Converts a graph after Graphviz layout/rendering into the UI-facing model.
    [[nodiscard]] LayoutResult extract_layout(Agraph_s* graph);
}
