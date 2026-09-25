/// @file
/// Implements presentation of language relations and witness words.
#include "ui/results/ComparisonResultView.hpp"

#include "ui/comparison/ComparisonDiagram.hpp"
#include "ui/operations/OperationState.hpp"
#include "ui/style/ApplicationTheme.hpp"

#include <algorithm>
#include <imgui.h>
#include <optional>

namespace ui::results
{
    namespace
    {
        /// Renders one comparison region, including an explanation when the region is empty.
        void render_witness(
            const std::optional<comparison::Witness>& witness,
            const char* fallback_label,
            const char* region_description,
            const ImVec4& color,
            const float width
        )
        {
            ImGui::PushID(fallback_label);
            ImGui::TextColored(
                color, "%s", witness.has_value() ? witness->label.c_str() : fallback_label
            );
            ImGui::SameLine();
            ImGui::TextUnformatted(region_description);

            if (witness.has_value())
            {
                ImVec4 surface = color;
                surface.w = 0.10F;
                ImVec4 border = color;
                border.w = 0.45F;
                ImGui::PushStyleColor(ImGuiCol_ChildBg, surface);
                ImGui::PushStyleColor(ImGuiCol_Border, border);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0F);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0F, 7.0F));
                if (ImGui::BeginChild(
                        "##witness_word",
                        ImVec2(width, 0.0F),
                        ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding |
                            ImGuiChildFlags_AutoResizeY
                    ))
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::TextWrapped("%s", witness->word.c_str());
                    ImGui::PopStyleColor();
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
            }
            else
            {
                ImGui::TextDisabled("Empty region — no witness exists.");
            }
            ImGui::Dummy(ImVec2(0.0F, 4.0F));
            ImGui::PopID();
        }

        /// Renders all available witnesses within a fixed wrapping width.
        void render_witnesses(const comparison::ComparisonModel& model, float width)
        {
            ImGui::BeginGroup();
            ImGui::Dummy(ImVec2(width, 0.0f));
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + width);
            const style::ApplicationPalette& palette = style::application_palette();
            render_witness(model.only_left, "w1", "first language only", palette.primary, width);
            render_witness(model.intersection, "w2", "both languages", palette.success, width);
            render_witness(model.only_right, "w3", "second language only", palette.accent, width);
            render_witness(model.neither, "w4", "neither language", palette.warning, width);
            ImGui::PopTextWrapPos();
            ImGui::EndGroup();
        }
    }

    void render_comparison_result(const operations::ComparisonOutput& output)
    {
        ImGui::TextWrapped("%s", output.message.c_str());
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        const float available_width = ImGui::GetContentRegionAvail().x;
        constexpr float ColumnSpacing = 28.0f;
        constexpr float PreferredWitnessWidth = 260.0f;
        constexpr float MaximumDiagramWidth = 400.0f;
        constexpr float MinimumSideBySideWidth = 520.0f;

        const bool use_columns = available_width >= MinimumSideBySideWidth;
        const float diagram_width =
            std::min(use_columns ? available_width * 0.55f : available_width, MaximumDiagramWidth);
        const float diagram_height = diagram_width * 0.8f;
        const float witness_width =
            use_columns
                ? std::min(PreferredWitnessWidth, available_width - diagram_width - ColumnSpacing)
                : diagram_width;
        const float group_width =
            use_columns ? diagram_width + ColumnSpacing + witness_width : diagram_width;

        ImGui::SetCursorPosX(
            ImGui::GetCursorPosX() + std::max(0.0f, (available_width - group_width) * 0.5f)
        );
        ImGui::BeginGroup();

        ImGui::BeginGroup();
        comparison::render_comparison_diagram(
            output.comparison, ImVec2(diagram_width, diagram_height)
        );
        ImGui::EndGroup();

        if (use_columns)
        {
            ImGui::SameLine(0.0f, ColumnSpacing);
        }
        else
        {
            ImGui::Dummy(ImVec2(0.0f, 10.0f));
        }
        render_witnesses(output.comparison, witness_width);
        ImGui::EndGroup();
    }
}
