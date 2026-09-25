/// @file
/// Implements the settings window and normalization feedback.
#include "ui/settings/SettingsPage.hpp"

#include "app/settings/ApplicationSettings.hpp"
#include "ui/input/RandomRegexControls.hpp"

#include <imgui.h>

namespace ui::settings
{
    bool render_settings_page(
        app::settings::ApplicationSettings& settings, bool& visible, const bool focus_requested
    )
    {
        if (!visible)
        {
            return false;
        }

        if (focus_requested)
        {
            ImGui::SetNextWindowFocus();
        }
        ImGui::SetNextWindowSizeConstraints(ImVec2(560.0F, 420.0F), ImVec2(900.0F, 900.0F));
        ImGui::SetNextWindowSize(ImVec2(600.0F, 650.0F), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Settings", &visible, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::End();
            return false;
        }

        bool changed = false;
        ImGui::TextUnformatted("Operations");
        ImGui::Separator();
        changed |= ImGui::SliderInt(
            "Timeout (seconds)",
            &settings.operation_timeout_seconds,
            app::settings::MinimumOperationTimeoutSeconds,
            app::settings::MaximumOperationTimeoutSeconds
        );
        ImGui::TextDisabled("Translate, rewrite and compare share this deadline.");

        ImGui::Dummy(ImVec2(0.0F, 8.0F));
        ImGui::TextUnformatted("Display");
        ImGui::Separator();
        changed |= ImGui::SliderFloat(
            "Graph height",
            &settings.graph_canvas_height,
            app::settings::MinimumGraphCanvasHeight,
            app::settings::MaximumGraphCanvasHeight,
            "%.0f px"
        );
        bool dark_mode = settings.color_theme == app::settings::ColorTheme::Dark;
        if (ImGui::Checkbox("Dark mode", &dark_mode))
        {
            settings.color_theme =
                dark_mode ? app::settings::ColorTheme::Dark : app::settings::ColorTheme::Light;
            changed = true;
        }

        ImGui::Dummy(ImVec2(0.0F, 8.0F));
        ImGui::TextUnformatted("Random finite-word regex generator");
        ImGui::Separator();
        changed |= ui::input::render_random_regex_config(settings.random_regex);

        ImGui::Dummy(ImVec2(0.0F, 8.0F));
        ImGui::TextUnformatted("Random omega regex generator");
        ImGui::Separator();
        changed |= ui::input::render_random_omega_regex_config(settings.random_omega_regex);


        ImGui::Separator();
        if (ImGui::Button("Reset all defaults"))
        {
            settings = app::settings::defaults();
            changed = true;
        }

        ImGui::End();

        if (changed)
        {
            app::settings::normalize(settings);
        }
        return changed;
    }
}
