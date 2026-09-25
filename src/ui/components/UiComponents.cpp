// Implements reusable visual building blocks for the application shell.
#include "ui/components/UiComponents.hpp"

#include "ui/style/ApplicationTheme.hpp"

#include <algorithm>

namespace ui::components
{
    bool begin_centered_card(const char* id, const float maximum_width)
    {
        const float available_width = ImGui::GetContentRegionAvail().x;
        const float width = std::max(1.0F, std::min(maximum_width, available_width));
        ImGui::SetCursorPosX(
            ImGui::GetCursorPosX() + std::max(0.0F, (available_width - width) * 0.5F)
        );

        const style::ApplicationPalette& palette = style::application_palette();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, palette.card);
        ImGui::PushStyleColor(ImGuiCol_Border, palette.border);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0F);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0F);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0F, 22.0F));

        return ImGui::BeginChild(
            id,
            ImVec2(width, 0.0F),
            ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding |
                ImGuiChildFlags_AutoResizeY
        );
    }

    void end_centered_card()
    {
        ImGui::EndChild();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
    }

    bool tab_button(const char* label, const bool active, const ImVec2& size)
    {
        const style::ApplicationPalette& palette = style::application_palette();
        const ImVec4 transparent(0.0F, 0.0F, 0.0F, 0.0F);
        ImGui::PushStyleColor(ImGuiCol_Button, transparent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, palette.info_surface);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, palette.info_surface);
        ImGui::PushStyleColor(
            ImGuiCol_Text,
            active ? palette.primary : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)
        );
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0F);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0F);
        const bool clicked = ImGui::Button(label, size);
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);

        if (active)
        {
            const ImVec2 minimum = ImGui::GetItemRectMin();
            const ImVec2 maximum = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(minimum.x, maximum.y - 1.0F),
                ImVec2(maximum.x, maximum.y - 1.0F),
                ImGui::GetColorU32(palette.primary),
                3.0F
            );
        }
        return clicked;
    }

    bool segmented_button(const char* label, const bool active, const ImVec2& size)
    {
        const style::ApplicationPalette& palette = style::application_palette();
        if (active)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, palette.primary);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, palette.primary_hovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, palette.primary_active);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0F, 1.0F, 1.0F, 1.0F));
        }

        const bool clicked = ImGui::Button(label, size);
        if (active)
        {
            ImGui::PopStyleColor(4);
        }
        return clicked;
    }

    bool primary_button(const char* label, const ImVec2& size)
    {
        const style::ApplicationPalette& palette = style::application_palette();
        ImGui::PushStyleColor(ImGuiCol_Button, palette.primary);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, palette.primary_hovered);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, palette.primary_active);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0F, 1.0F, 1.0F, 1.0F));
        const bool clicked = ImGui::Button(label, size);
        ImGui::PopStyleColor(4);
        return clicked;
    }

    void badge(const char* label, const ImVec4& color)
    {
        const ImVec2 text_size = ImGui::CalcTextSize(label);
        constexpr float HorizontalPadding = 8.0F;
        constexpr float VerticalPadding = 3.0F;
        const ImVec2 minimum = ImGui::GetCursorScreenPos();
        const ImVec2 maximum(
            minimum.x + text_size.x + HorizontalPadding * 2.0F,
            minimum.y + text_size.y + VerticalPadding * 2.0F
        );

        ImVec4 background = color;
        background.w = 0.14F;
        ImGui::GetWindowDrawList()->AddRectFilled(
            minimum, maximum, ImGui::GetColorU32(background), 5.0F
        );
        ImGui::GetWindowDrawList()->AddRect(
            minimum, maximum, ImGui::GetColorU32(color), 5.0F, 0, 1.0F
        );
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(minimum.x + HorizontalPadding, minimum.y + VerticalPadding),
            ImGui::GetColorU32(color),
            label
        );
        ImGui::Dummy(ImVec2(maximum.x - minimum.x, maximum.y - minimum.y));
    }

    void section_heading(const char* label, const char* description)
    {
        ImGui::TextUnformatted(label);
        if (description != nullptr)
        {
            ImGui::TextDisabled("%s", description);
        }
    }
}
