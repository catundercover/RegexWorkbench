/// @file
/// Implements graph result status, controls, statistics, and canvas presentation.
#include "ui/results/GraphResultView.hpp"

#include "app/platform/PlatformIntegration.hpp"
#include "graph/model/Layout.hpp"
#include "ui/components/UiComponents.hpp"
#include "ui/graph/AutomatonGraphCanvas.hpp"
#include "ui/operations/OperationState.hpp"
#include "ui/results/ResultWidgets.hpp"
#include "ui/style/ApplicationTheme.hpp"

#include <algorithm>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <string>
#include <unordered_set>

namespace ui::results
{
    namespace
    {
        /// Counts rendered automaton states without the synthetic start marker.
        std::size_t state_count(const ::graph::Layout& graph)
        {
            return static_cast<std::size_t>(std::ranges::count_if(
                graph.nodes,
                [](const ::graph::Node& node) { return node.role == ::graph::NodeRole::State; }
            ));
        }

        /// Counts automaton transitions without the synthetic initial arrow.
        std::size_t transition_count(const ::graph::Layout& graph)
        {
            std::unordered_set<std::string> start_markers;
            for (const ::graph::Node& node : graph.nodes)
            {
                if (node.role == ::graph::NodeRole::StartMarker)
                {
                    start_markers.insert(node.id);
                }
            }

            return static_cast<std::size_t>(std::ranges::count_if(
                graph.edges,
                [&start_markers](const ::graph::Edge& edge)
                { return !start_markers.contains(edge.source); }
            ));
        }

        /// Renders graph/text navigation and actions for the active view.
        void render_toolbar(operations::GraphOutput& output)
        {
            constexpr float TabWidth = 86.0F;
            if (components::tab_button(
                    "Graph", !output.text_view_visible, ImVec2(TabWidth, ImGui::GetFrameHeight())
                ))
            {
                output.text_view_visible = false;
            }
            ImGui::SameLine();
            if (components::tab_button(
                    "Text", output.text_view_visible, ImVec2(TabWidth, ImGui::GetFrameHeight())
                ))
            {
                output.text_view_visible = true;
            }

            if (output.text_view_visible)
            {
                ImGui::SameLine();
                const bool copy_clicked = ImGui::Button("Copy");
                const ImVec2 copy_minimum = ImGui::GetItemRectMin();
                const ImVec2 copy_maximum = ImGui::GetItemRectMax();
                app::platform::register_browser_clipboard_target(
                    copy_minimum.x,
                    copy_minimum.y,
                    copy_maximum.x,
                    copy_maximum.y,
                    output.automaton_text.c_str()
                );
                if (copy_clicked)
                {
                    ImGui::SetClipboardText(output.automaton_text.c_str());
                }
            }
            else
            {
                ImGui::SameLine();
                if (ImGui::Button("Fit view"))
                {
                    ui::graph::fit_graph_view(output.canvas);
                }

                ImGui::SameLine();
                ImGui::BeginDisabled(ui::graph::is_initial_graph_view(output.canvas));
                if (ImGui::Button("Reset layout"))
                {
                    output.graph = output.original_graph;
                    ui::graph::reset_graph(output.canvas);
                }
                ImGui::EndDisabled();
            }
        }
    }

    void render_graph_result(operations::GraphOutput& output, const float canvas_height)
    {
        ImGui::TextDisabled(
            "%zu states  -  %zu transitions",
            state_count(output.graph),
            transition_count(output.graph)
        );

        render_toolbar(output);

        if (output.text_view_visible)
        {
            ImGui::TextDisabled("Formal automaton definition");
            ImGui::InputTextMultiline(
                "##automaton_text",
                &output.automaton_text,
                ImVec2(-1.0F, canvas_height),
                ImGuiInputTextFlags_ReadOnly
            );
            return;
        }

        ImGui::PushID("graph_view_controls");
        if (ImGui::SmallButton("-"))
        {
            ui::graph::zoom_graph(output.canvas, -1.0F);
        }
        ImGui::SameLine();
        ImGui::Text("%.0f%%", output.canvas.zoom * 100.0F);
        ImGui::SameLine();
        if (ImGui::SmallButton("+"))
        {
            ui::graph::zoom_graph(output.canvas, 1.0F);
        }
        ImGui::PopID();
        ImGui::SameLine();
        ImGui::TextDisabled("Drag states to move; drag the background to pan; scroll to zoom.");

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ui::style::application_palette().subtle_surface);
        ui::graph::render_automaton_graph(
            output.graph, output.canvas, ImVec2(0.0F, canvas_height), "##automaton_graph"
        );
        ImGui::PopStyleColor();
    }
}
