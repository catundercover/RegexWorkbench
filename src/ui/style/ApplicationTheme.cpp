/// @file
/// Implements the supported application-wide Dear ImGui palettes.
#include "ui/style/ApplicationTheme.hpp"

#include <imgui.h>

namespace ui::style
{
    namespace
    {
        constexpr ApplicationPalette LightPalette{
            {0.10F, 0.42F, 0.82F, 1.00F},
            {0.08F, 0.36F, 0.73F, 1.00F},
            {0.06F, 0.30F, 0.64F, 1.00F},
            {0.61F, 0.18F, 0.72F, 1.00F},
            {0.08F, 0.55F, 0.33F, 1.00F},
            {0.78F, 0.48F, 0.06F, 1.00F},
            {0.82F, 0.20F, 0.25F, 1.00F},
            {1.00F, 1.00F, 1.00F, 1.00F},
            {0.95F, 0.96F, 0.98F, 1.00F},
            {0.91F, 0.95F, 1.00F, 1.00F},
            {0.91F, 0.98F, 0.94F, 1.00F},
            {1.00F, 0.93F, 0.94F, 1.00F},
            {0.82F, 0.85F, 0.90F, 1.00F}
        };

        constexpr ApplicationPalette DarkPalette{
            {0.31F, 0.65F, 1.00F, 1.00F},
            {0.39F, 0.70F, 1.00F, 1.00F},
            {0.22F, 0.56F, 0.92F, 1.00F},
            {0.82F, 0.48F, 0.91F, 1.00F},
            {0.29F, 0.78F, 0.52F, 1.00F},
            {0.96F, 0.68F, 0.25F, 1.00F},
            {1.00F, 0.42F, 0.46F, 1.00F},
            {0.085F, 0.10F, 0.13F, 1.00F},
            {0.11F, 0.13F, 0.17F, 1.00F},
            {0.10F, 0.17F, 0.25F, 1.00F},
            {0.09F, 0.20F, 0.15F, 1.00F},
            {0.23F, 0.10F, 0.12F, 1.00F},
            {0.20F, 0.24F, 0.31F, 1.00F}
        };

        ApplicationPalette CurrentPalette = LightPalette;
    }

    const ApplicationPalette& application_palette() noexcept
    {
        return CurrentPalette;
    }

    void apply_application_theme(const app::settings::ColorTheme theme)
    {
        const bool dark = theme == app::settings::ColorTheme::Dark;
        CurrentPalette = dark ? DarkPalette : LightPalette;
        if (dark)
        {
            ImGui::StyleColorsDark();
        }
        else
        {
            ImGui::StyleColorsLight();
        }

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowBorderSize = 0.0F;
        style.ChildBorderSize = 1.0F;
        style.PopupBorderSize = 1.0F;
        style.WindowPadding = ImVec2(24.0F, 20.0F);
        style.FrameRounding = 7.0F;
        style.ChildRounding = 10.0F;
        style.PopupRounding = 8.0F;
        style.ScrollbarRounding = 8.0F;
        style.GrabRounding = 6.0F;
        style.FrameBorderSize = 1.0F;
        style.FramePadding = ImVec2(10.0F, 7.0F);
        style.ItemSpacing = ImVec2(10.0F, 10.0F);
        style.ItemInnerSpacing = ImVec2(7.0F, 6.0F);
        style.SeparatorSize = 2.0F;

        if (dark)
        {
            style.Colors[ImGuiCol_Text] = ImVec4(0.91F, 0.93F, 0.97F, 1.00F);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.56F, 0.61F, 0.69F, 1.00F);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.055F, 0.067F, 0.09F, 1.00F);
            style.Colors[ImGuiCol_ChildBg] = CurrentPalette.card;
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0.075F, 0.09F, 0.12F, 0.99F);
            style.Colors[ImGuiCol_Border] = CurrentPalette.border;
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.07F, 0.085F, 0.115F, 1.00F);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.10F, 0.13F, 0.18F, 1.00F);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.12F, 0.16F, 0.22F, 1.00F);
        }
        else
        {
            style.Colors[ImGuiCol_Text] = ImVec4(0.10F, 0.13F, 0.18F, 1.00F);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.43F, 0.47F, 0.54F, 1.00F);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.955F, 0.965F, 0.98F, 1.00F);
            style.Colors[ImGuiCol_ChildBg] = CurrentPalette.card;
            style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00F, 1.00F, 1.00F, 0.99F);
            style.Colors[ImGuiCol_Border] = CurrentPalette.border;
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.985F, 0.99F, 1.00F, 1.00F);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95F, 0.97F, 1.00F, 1.00F);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.92F, 0.95F, 1.00F, 1.00F);
        }

        style.Colors[ImGuiCol_Button] = CurrentPalette.subtle_surface;
        style.Colors[ImGuiCol_ButtonHovered] = CurrentPalette.info_surface;
        style.Colors[ImGuiCol_ButtonActive] = CurrentPalette.primary;
        style.Colors[ImGuiCol_Header] = CurrentPalette.info_surface;
        style.Colors[ImGuiCol_HeaderHovered] = CurrentPalette.info_surface;
        style.Colors[ImGuiCol_HeaderActive] = CurrentPalette.primary;
        style.Colors[ImGuiCol_CheckMark] = CurrentPalette.primary;
        style.Colors[ImGuiCol_SliderGrab] = CurrentPalette.primary;
        style.Colors[ImGuiCol_SliderGrabActive] = CurrentPalette.primary_active;
        style.Colors[ImGuiCol_Separator] = CurrentPalette.border;
        style.Colors[ImGuiCol_ResizeGrip] = CurrentPalette.primary;
        style.Colors[ImGuiCol_NavCursor] = CurrentPalette.primary;
    }
}
