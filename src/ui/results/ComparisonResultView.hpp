/// @file
/// Declares presentation of language-comparison results.
#pragma once

namespace ui::operations
{
    struct ComparisonOutput;
}

namespace ui::results
{
    /// Renders a comparison description, diagram, and available witnesses.
    void render_comparison_result(const operations::ComparisonOutput& output);
}
