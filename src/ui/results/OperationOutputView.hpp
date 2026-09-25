/// @file
/// Declares variant-based presentation of operation outputs.
#pragma once

#include "ui/operations/OperationState.hpp"
#include "ui/results/ResultAction.hpp"

namespace ui::results
{
    /// Renders the current operation status or result; reports a requested cancellation.
    [[nodiscard]] ResultAction render_operation_output(
        operations::OperationOutput& output, bool operation_running, float graph_canvas_height
    );
}
