// Declares the set diagram used to visualize language comparisons.
#pragma once

struct ImVec2;

namespace ui::comparison
{
    struct ComparisonModel;

    // Renders the language relation as a compact set diagram.
    void render_comparison_diagram(const ComparisonModel& comparison, const ImVec2& size);
}
