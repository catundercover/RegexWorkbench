/// @file
/// Declares presentation of rewritten finite and omega expressions.
#pragma once

namespace ui::operations
{
    struct RewriteOutput;
}

namespace ui::results
{
    /// Renders the parseable rewritten finite or omega regular expression.
    void render_rewrite_result(const operations::RewriteOutput& output);
}
