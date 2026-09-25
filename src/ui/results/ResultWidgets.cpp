/// @file
/// Implements status and progress widgets shared by result views.
#include "ui/results/ResultWidgets.hpp"

#include "ui/style/ApplicationTheme.hpp"

#include <cmath>
#include <imgui.h>
#include <string>

namespace ui::results
{
    namespace
    {
        /// Draws a time-animated partial circle at the current cursor.
        void render_spinner()
        {
            constexpr float Radius = 7.0F;
            constexpr float Thickness = 2.0F;
            constexpr float Tau = 6.28318530718F;

            const ImVec2 cursor = ImGui::GetCursorScreenPos();
            const ImVec2 center{cursor.x + Radius, cursor.y + ImGui::GetTextLineHeight() * 0.5F};
            const float start = static_cast<float>(std::fmod(ImGui::GetTime() * 4.0, Tau));
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->PathArcTo(center, Radius, start, start + Tau * 0.75F, 24);
            draw_list->PathStroke(
                ImGui::GetColorU32(style::application_palette().primary), 0, Thickness
            );
            ImGui::Dummy(ImVec2(Radius * 2.0F, ImGui::GetTextLineHeight()));
        }
    }

    void render_error_message(const std::string_view message)
    {
        if (message.empty())
        {
            return;
        }

        const style::ApplicationPalette& palette = style::application_palette();

        ImGui::PushID(message.data(), message.data() + message.size());
        ImGui::PushStyleColor(ImGuiCol_ChildBg, palette.error_surface);
        ImGui::PushStyleColor(ImGuiCol_Border, palette.error);
        ImGui::PushStyleColor(ImGuiCol_Text, palette.error);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0F, 9.0F));
        ImGui::BeginChild(
            "##status_message",
            ImVec2(0.0F, 0.0F),
            ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding |
                ImGuiChildFlags_AutoResizeY
        );
        const std::string text(message);
        ImGui::TextWrapped("%s", text.c_str());
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }

    bool render_activity(const std::string_view label, const char* cancel_label)
    {
        render_spinner();
        ImGui::SameLine();
        const std::string text(label);
        ImGui::TextColored(style::application_palette().primary, "%s", text.c_str());
        if (cancel_label == nullptr)
        {
            return false;
        }

        ImGui::SameLine();
        return ImGui::Button(cancel_label);
    }
}
