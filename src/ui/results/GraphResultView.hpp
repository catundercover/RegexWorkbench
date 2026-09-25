/// @file
/// Declares presentation and interaction for automaton graph results.
#pragma once

namespace ui::operations
{
    struct GraphOutput;
}

namespace ui::results
{
    /// Renders a translated automaton and its layout canvas.
    void render_graph_result(operations::GraphOutput& output, float canvas_height);
}
