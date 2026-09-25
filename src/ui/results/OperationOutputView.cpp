/// @file
/// Implements dispatch from operation-output variants to result views.
#include "ui/results/OperationOutputView.hpp"

#include "ui/results/ComparisonResultView.hpp"
#include "ui/results/GraphResultView.hpp"
#include "ui/results/ResultWidgets.hpp"
#include "ui/results/RewriteResultView.hpp"

#include <imgui.h>
#include <variant>

namespace ui::results
{
    ResultAction render_operation_output(
        operations::OperationOutput& output,
        const bool operation_running,
        const float graph_canvas_height
    )
    {
        if (std::holds_alternative<std::monostate>(output))
        {
            ImGui::Dummy(ImVec2(0.0F, 14.0F));
            const char* title = "Your result will appear here";
            const char* hint = "Enter an expression, choose an operation, and press Run.";
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() +
                (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(title).x) * 0.5F
            );
            ImGui::TextUnformatted(title);
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() +
                (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(hint).x) * 0.5F
            );
            ImGui::TextDisabled("%s", hint);
            ImGui::Dummy(ImVec2(0.0F, 18.0F));
            return {};
        }

        if (const auto* pending = std::get_if<operations::PendingOutput>(&output))
        {
            if (!operation_running)
            {
                return {};
            }
            if (!activity_delay_elapsed(pending->activity_started_at))
            {
                return {};
            }

            ImGui::Dummy(ImVec2(0.0f, 10.0f));
            return render_activity(pending->label, "Cancel")
                       ? ResultAction{ResultActionKind::CancelOperation}
                       : ResultAction{};
        }

        if (const auto* failure = std::get_if<operations::FailureOutput>(&output))
        {
            render_error_message(failure->message);
            return {};
        }

        if (auto* graph = std::get_if<operations::GraphOutput>(&output))
        {
            render_graph_result(*graph, graph_canvas_height);
            return {};
        }

        if (const auto* rewrite = std::get_if<operations::RewriteOutput>(&output))
        {
            render_rewrite_result(*rewrite);
            return {};
        }

        if (const auto* comparison = std::get_if<operations::ComparisonOutput>(&output))
        {
            render_comparison_result(*comparison);
        }

        return {};
    }
}
