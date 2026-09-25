// Declares reusable visual building blocks for the application shell.
#pragma once

#include <imgui.h>

namespace ui::components
{
    // Preferred width of the focused operation workspace.
    inline constexpr float ControlCardWidth = 920.0F;
    /// Preferred maximum width of result content, including automaton graphs.
    inline constexpr float ResultCardWidth = 1400.0F;

    // Begins a horizontally centered, auto-height surface card.
    [[nodiscard]] bool begin_centered_card(const char* id, float maximum_width);

    // Ends a card started with `begin_centered_card`.
    void end_centered_card();

    // Renders a text tab with an active underline.
    [[nodiscard]] bool tab_button(const char* label, bool active, const ImVec2& size);

    // Renders one choice in a compact segmented selector.
    [[nodiscard]] bool segmented_button(const char* label, bool active, const ImVec2& size);

    // Renders the application-wide emphasized action button.
    [[nodiscard]] bool primary_button(const char* label, const ImVec2& size = ImVec2());

    // Renders a compact semantic label with a tinted background.
    void badge(const char* label, const ImVec4& color);

    // Renders an uppercase-style section label and optional explanatory copy.
    void section_heading(const char* label, const char* description = nullptr);
}
