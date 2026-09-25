/// @file
/// Declares application-wide Dear ImGui theme application.
#pragma once

#include "app/settings/ApplicationSettings.hpp"

#include <imgui.h>

namespace ui::style
{
    /// Semantic colors shared by custom application widgets.
    struct ApplicationPalette
    {
        ImVec4 primary;
        ImVec4 primary_hovered;
        ImVec4 primary_active;
        ImVec4 accent;
        ImVec4 success;
        ImVec4 warning;
        ImVec4 error;
        ImVec4 card;
        ImVec4 subtle_surface;
        ImVec4 info_surface;
        ImVec4 success_surface;
        ImVec4 error_surface;
        ImVec4 border;
    };

    /// Returns the semantic palette installed by the latest theme application.
    [[nodiscard]] const ApplicationPalette& application_palette() noexcept;

    /// Applies the selected application-wide Dear ImGui palette and dimensions.
    void apply_application_theme(app::settings::ColorTheme theme);
}
