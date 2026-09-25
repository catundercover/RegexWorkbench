/// @file
/// Implements copyable presentation of rewritten finite and omega expressions.
#include "ui/results/RewriteResultView.hpp"

#include "app/operations/ExpressionFlavor.hpp"
#include "app/platform/PlatformIntegration.hpp"
#include "ui/components/UiComponents.hpp"
#include "ui/operations/OperationState.hpp"
#include "ui/style/ApplicationTheme.hpp"

#include <imgui.h>

namespace ui::results
{
    void render_rewrite_result(const operations::RewriteOutput& output)
    {
        const bool omega = output.flavor == app::operations::ExpressionFlavor::OmegaRegex;
        ImGui::TextUnformatted(
            omega ? "Rewritten omega regular expression" : "Rewritten regular expression"
        );
        ImGui::Dummy(ImVec2(0.0F, 4.0F));

        const style::ApplicationPalette& palette = style::application_palette();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, palette.subtle_surface);
        ImGui::PushStyleColor(ImGuiCol_Border, palette.border);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0F, 12.0F));
        ImGui::BeginChild(
            "##rewritten_expression",
            ImVec2(0.0F, 0.0F),
            ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding |
                ImGuiChildFlags_AutoResizeY
        );
        ImGui::TextWrapped("%s", output.regex.c_str());
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);

        const float button_width = 150.0F;
        ImGui::SetCursorPosX(
            ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - button_width
        );
        const bool copy_clicked =
            components::primary_button("Copy expression", ImVec2(button_width, 0.0F));
        const ImVec2 copy_minimum = ImGui::GetItemRectMin();
        const ImVec2 copy_maximum = ImGui::GetItemRectMax();
        app::platform::register_browser_clipboard_target(
            copy_minimum.x, copy_minimum.y, copy_maximum.x, copy_maximum.y, output.regex.c_str()
        );
        if (copy_clicked)
        {
            ImGui::SetClipboardText(output.regex.c_str());
        }
    }
}
