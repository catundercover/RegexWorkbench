// Implements interactive rendering of automaton graph layouts.
#include "ui/graph/AutomatonGraphCanvas.hpp"

#include "graph/model/Layout.hpp"

#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ui::graph
{
    namespace
    {
        // Epsilon used to reject unstable normalization and scale calculations.
        constexpr float MinimumVectorLength = 0.001f;
        // Default automaton-state fill color.
        constexpr ImU32 NodeColor = IM_COL32(66, 150, 250, 255);
        // Fill color for a node under the pointer.
        constexpr ImU32 HoveredNodeColor = IM_COL32(86, 168, 255, 255);
        // Fill color for a node being dragged.
        constexpr ImU32 ActiveNodeColor = IM_COL32(37, 126, 225, 255);

        // Converts between Graphviz layout coordinates and canvas screen coordinates.
        struct CanvasTransform
        {
            // Fits layout bounds to the canvas, then applies user zoom and pan.
            explicit CanvasTransform(
                const ::graph::Layout& graph_layout,
                const GraphCanvasState& state,
                const ImVec2& canvas_position,
                const ImVec2& canvas_size
            )
            {
                const float graph_width =
                    std::max(1.0f, graph_layout.bounds.maximum.x - graph_layout.bounds.minimum.x);
                const float graph_height =
                    std::max(1.0f, graph_layout.bounds.maximum.y - graph_layout.bounds.minimum.y);

                const float fitted_scale =
                    std::min(canvas_size.x / graph_width, canvas_size.y / graph_height);
                scale = std::max(MinimumVectorLength, fitted_scale * state.zoom);
                screen_center = ImVec2(
                    canvas_position.x + canvas_size.x * 0.5F,
                    canvas_position.y + canvas_size.y * 0.5F
                );
                view_center = ::graph::Point{
                    (graph_layout.bounds.minimum.x + graph_layout.bounds.maximum.x) * 0.5F +
                        state.view_offset.x,
                    (graph_layout.bounds.minimum.y + graph_layout.bounds.maximum.y) * 0.5F +
                        state.view_offset.y
                };
            }

            // Projects a layout point into screen space.
            [[nodiscard]] ImVec2 operator()(const ::graph::Point& point) const
            {
                return ImVec2(
                    screen_center.x + (point.x - view_center.x) * scale,
                    screen_center.y - (point.y - view_center.y) * scale
                );
            }

            // Projects a screen-space point back into layout coordinates.
            [[nodiscard]] ::graph::Point to_layout(const ImVec2& point) const
            {
                return {
                    view_center.x + (point.x - screen_center.x) / scale,
                    view_center.y - (point.y - screen_center.y) / scale
                };
            }

            ImVec2 screen_center;
            ::graph::Point view_center;
            float scale = 1.0f;
        };

        // Visible straight edge between two node circumferences.
        struct LineSegment
        {
            ImVec2 start;
            ImVec2 end;
            ImVec2 direction;
        };

        // Per-frame hover and drag state used when drawing one node.
        struct NodeVisualState
        {
            bool hovered = false;
            bool active = false;
        };

        // Collects per-node visual state and whether a node owns the current drag.
        struct NodeInteraction
        {
            std::vector<NodeVisualState> visual_states;
            bool any_active = false;
        };

        // Maps stable graph node identifiers to layout-vector indices.
        using NodeIndex = std::unordered_map<std::string, std::size_t>;

        // Returns the current theme color for the canvas background.
        [[nodiscard]] ImU32 canvas_color()
        {
            return ImGui::GetColorU32(ImGuiCol_ChildBg);
        }

        // Returns the current theme color for graph lines and labels.
        [[nodiscard]] ImU32 line_color()
        {
            return ImGui::GetColorU32(ImGuiCol_Text);
        }

        // Claims mouse-wheel input for the canvas instead of its parent window.
        void capture_graph_wheel(const ImVec2& canvas_position, const ImVec2& canvas_size)
        {
            if (canvas_size.x <= 0.0F || canvas_size.y <= 0.0F)
            {
                return;
            }

            const ImVec2 saved_cursor = ImGui::GetCursorScreenPos();
            ImGui::SetCursorScreenPos(canvas_position);
            ImGui::SetNextItemAllowOverlap();
            // Restrict the capture item to the middle button so it cannot steal left-click drags.
            ImGui::InvisibleButton(
                "##graph_wheel_capture", canvas_size, ImGuiButtonFlags_MouseButtonMiddle
            );
            static_cast<void>(ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelX));
            static_cast<void>(ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY));
            ImGui::SetCursorScreenPos(saved_cursor);
        }

        // Applies zoom while keeping the graph point beneath the pointer stationary.
        void apply_mouse_wheel_zoom(
            const ::graph::Layout& layout,
            GraphCanvasState& state,
            const ImVec2& canvas_position,
            const ImVec2& canvas_size
        )
        {
            const float wheel_steps = ImGui::GetIO().MouseWheel;
            if (!ImGui::IsWindowHovered() || wheel_steps == 0.0F)
            {
                return;
            }

            const ImVec2 mouse_position = ImGui::GetIO().MousePos;
            const CanvasTransform before(layout, state, canvas_position, canvas_size);
            const ::graph::Point anchor = before.to_layout(mouse_position);
            const float previous_zoom = state.zoom;
            zoom_graph(state, wheel_steps);
            if (state.zoom == previous_zoom)
            {
                return;
            }

            const CanvasTransform after(layout, state, canvas_position, canvas_size);
            const ::graph::Point moved_anchor = after.to_layout(mouse_position);
            state.view_offset.x += anchor.x - moved_anchor.x;
            state.view_offset.y += anchor.y - moved_anchor.y;
        }

        /// Keeps the viewport center within the part of the layout hidden by zoom.
        void clamp_view_to_layout(
            const ::graph::Layout& layout,
            GraphCanvasState& state,
            const ImVec2& canvas_size,
            const float scale
        )
        {
            const float graph_width =
                std::max(1.0F, layout.bounds.maximum.x - layout.bounds.minimum.x);
            const float graph_height =
                std::max(1.0F, layout.bounds.maximum.y - layout.bounds.minimum.y);
            const ::graph::Point maximum_offset{
                std::max(0.0F, graph_width * 0.5F - canvas_size.x / (2.0F * scale)),
                std::max(0.0F, graph_height * 0.5F - canvas_size.y / (2.0F * scale))
            };
            clamp_graph_view(state, maximum_offset);
        }

        // Pans the viewport when the user drags empty canvas space.
        [[nodiscard]] bool
        apply_background_pan(GraphCanvasState& state, const float scale, const bool node_active)
        {
            if (node_active)
            {
                state.background_drag_active = false;
                return false;
            }

            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                state.background_drag_active = false;
                return false;
            }

            if (!state.background_drag_active)
            {
                if (!ImGui::IsWindowHovered() || !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    return false;
                }
                state.background_drag_active = true;
            }

            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
            if (state.zoom <= 1.0F)
            {
                return false;
            }

            const ImVec2 delta = ImGui::GetIO().MouseDelta;
            if (delta.x == 0.0F && delta.y == 0.0F)
            {
                return false;
            }

            pan_graph(state, {delta.x, delta.y}, scale);
            return true;
        }

        // Subtracts two screen-space vectors.
        [[nodiscard]] ImVec2 difference(const ImVec2& lhs, const ImVec2& rhs)
        {
            return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
        }

        // Converts a Dear ImGui vector to a graph point without changing coordinates.
        [[nodiscard]] ::graph::Point as_point(const ImVec2& value)
        {
            return {value.x, value.y};
        }

        // Converts a graph point to a Dear ImGui vector without changing coordinates.
        [[nodiscard]] ImVec2 as_vector(const ::graph::Point value)
        {
            return {value.x, value.y};
        }

        // Draws a compact overview of the graph and the currently visible layout region.
        void draw_minimap(
            ImDrawList* draw_list,
            const ::graph::Layout& layout,
            const GraphCanvasState& state,
            const CanvasTransform& transform,
            const ImVec2& canvas_position,
            const ImVec2& canvas_size
        )
        {
            if (state.zoom <= 1.0F)
            {
                return;
            }

            constexpr float Margin = 10.0F;
            constexpr float Padding = 7.0F;
            const float extent = std::min(120.0F, std::min(canvas_size.x, canvas_size.y) * 0.28F);
            if (extent < 48.0F)
            {
                return;
            }

            const ImVec2 outer_min{
                canvas_position.x + canvas_size.x - extent - Margin, canvas_position.y + Margin
            };
            const ImVec2 outer_max{outer_min.x + extent, outer_min.y + extent};
            const ImVec2 inner_min{outer_min.x + Padding, outer_min.y + Padding};
            const ImVec2 inner_max{outer_max.x - Padding, outer_max.y - Padding};

            draw_list->AddRectFilled(outer_min, outer_max, IM_COL32(85, 85, 92, 220), 5.0F);
            draw_list->AddRect(outer_min, outer_max, IM_COL32(190, 190, 198, 230), 5.0F, 0, 1.5F);

            const float graph_width =
                std::max(1.0F, layout.bounds.maximum.x - layout.bounds.minimum.x);
            const float graph_height =
                std::max(1.0F, layout.bounds.maximum.y - layout.bounds.minimum.y);
            const auto map_to_minimap = [&](const ::graph::Point point)
            {
                const float horizontal =
                    std::clamp((point.x - layout.bounds.minimum.x) / graph_width, 0.0F, 1.0F);
                const float vertical =
                    std::clamp((layout.bounds.maximum.y - point.y) / graph_height, 0.0F, 1.0F);
                return ImVec2{
                    inner_min.x + horizontal * (inner_max.x - inner_min.x),
                    inner_min.y + vertical * (inner_max.y - inner_min.y)
                };
            };

            const float half_visible_width = canvas_size.x / (2.0F * transform.scale);
            const float half_visible_height = canvas_size.y / (2.0F * transform.scale);
            const ::graph::Point visible_min{
                std::max(layout.bounds.minimum.x, transform.view_center.x - half_visible_width),
                std::max(layout.bounds.minimum.y, transform.view_center.y - half_visible_height)
            };
            const ::graph::Point visible_max{
                std::min(layout.bounds.maximum.x, transform.view_center.x + half_visible_width),
                std::min(layout.bounds.maximum.y, transform.view_center.y + half_visible_height)
            };
            const ImVec2 viewport_min = map_to_minimap({visible_min.x, visible_max.y});
            const ImVec2 viewport_max = map_to_minimap({visible_max.x, visible_min.y});
            draw_list->AddRectFilled(viewport_min, viewport_max, IM_COL32(235, 55, 55, 45), 1.0F);
            draw_list->AddRect(
                viewport_min, viewport_max, IM_COL32(245, 65, 65, 255), 1.0F, 0, 2.0F
            );
        }

        // Projects one graph point through a canvas transform.
        [[nodiscard]] ::graph::Point
        transformed_point(const ::graph::Point& point, const CanvasTransform& transform)
        {
            return as_point(transform(point));
        }

        // Projects every control point of a curve into screen space.
        [[nodiscard]] ::graph::CubicBezier
        transformed_segment(const ::graph::CubicBezier& segment, const CanvasTransform& transform)
        {
            return {
                transformed_point(segment.start, transform),
                transformed_point(segment.first_control, transform),
                transformed_point(segment.second_control, transform),
                transformed_point(segment.end, transform)
            };
        }

        // Returns the Euclidean length of a screen-space vector.
        [[nodiscard]] float length(const ImVec2& vector)
        {
            return std::sqrt(vector.x * vector.x + vector.y * vector.y);
        }

        // Returns a unit vector, or no value when its direction is unstable.
        [[nodiscard]] std::optional<ImVec2> normalized(const ImVec2& vector)
        {
            const float vector_length = length(vector);
            if (vector_length <= MinimumVectorLength)
            {
                return std::nullopt;
            }

            return ImVec2(vector.x / vector_length, vector.y / vector_length);
        }

        // Moves a point by a direction scaled to the supplied distance.
        [[nodiscard]] ImVec2
        translated(const ImVec2& point, const ImVec2& direction, float distance)
        {
            return ImVec2(point.x + direction.x * distance, point.y + direction.y * distance);
        }

        // Resolves a stable node identifier to its layout index.
        [[nodiscard]] std::optional<std::size_t>
        find_node_index(const NodeIndex& node_indices, const std::string& id)
        {
            const auto entry = node_indices.find(id);
            if (entry == node_indices.end())
            {
                return std::nullopt;
            }

            return entry->second;
        }

        // Clips a straight edge to its source and target node boundaries.
        [[nodiscard]] std::optional<LineSegment> make_line_segment(
            const ::graph::Node& from_node,
            const ::graph::Node& to_node,
            const ImVec2& from_position,
            const ImVec2& to_position
        )
        {
            const std::optional<ImVec2> direction =
                normalized(difference(to_position, from_position));
            if (!direction.has_value())
            {
                return std::nullopt;
            }

            const float start_offset =
                from_node.role == ::graph::NodeRole::StartMarker ? 0.0F : from_node.radius;
            const float end_offset =
                to_node.role == ::graph::NodeRole::StartMarker ? 0.0F : to_node.radius;
            return LineSegment{
                translated(from_position, *direction, start_offset),
                translated(to_position, *direction, -end_offset),
                *direction
            };
        }

        // Draws a filled arrowhead oriented along an edge direction.
        void draw_arrow_head(
            ImDrawList* draw_list, const ImVec2& tip, const ImVec2& direction, ImU32 color
        )
        {
            const std::optional<ImVec2> unit = normalized(direction);
            if (!unit.has_value())
            {
                return;
            }

            const ImVec2 normal(-unit->y, unit->x);
            const ImVec2 arrow_base = translated(tip, *unit, -12.0f);
            const ImVec2 left = translated(arrow_base, normal, 6.0f);
            const ImVec2 right = translated(arrow_base, normal, -6.0f);
            draw_list->AddTriangleFilled(tip, left, right, color);
        }

        // Draws a graph label centered on a screen-space position.
        void draw_label(ImDrawList* draw_list, const std::string& text, const ImVec2& position)
        {
            if (text.empty())
            {
                return;
            }

            const ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
            const ImVec2 label_position(
                position.x - text_size.x * 0.5f, position.y - text_size.y * 0.5f
            );
            draw_list->AddText(label_position, line_color(), text.c_str());
        }

        // Draws a stable loop, arrowhead, and label around one node.
        void draw_self_loop(
            ImDrawList* draw_list,
            const ImVec2& node_position,
            float node_radius,
            const std::string& label
        )
        {
            const detail::SelfLoopGeometry loop =
                detail::make_self_loop_geometry(as_point(node_position), node_radius);
            draw_list->AddBezierCubic(
                as_vector(loop.curve.start),
                as_vector(loop.curve.first_control),
                as_vector(loop.curve.second_control),
                as_vector(loop.curve.end),
                line_color(),
                2.0F,
                32
            );
            draw_arrow_head(
                draw_list, as_vector(loop.curve.end), as_vector(loop.arrow_direction), line_color()
            );
            draw_label(draw_list, label, as_vector(loop.label_position));
        }

        // Draws an untouched Graphviz spline.
        void draw_curved_edge(
            ImDrawList* draw_list,
            const ::graph::Edge& edge,
            const ::graph::Node& from_node,
            const ::graph::Node& to_node,
            const ImVec2& from_position,
            const ImVec2& to_position,
            const CanvasTransform& transform
        )
        {
            const int segment_count =
                std::max(1, static_cast<int>(ImGui::GetIO().DisplayFramebufferScale.x * 20.0f));

            std::optional<::graph::CubicBezier> final_segment;
            for (std::size_t index = 0; index < edge.spline.size(); ++index)
            {
                ::graph::CubicBezier segment = transformed_segment(edge.spline[index], transform);
                if (index == 0 && from_node.role == ::graph::NodeRole::State)
                {
                    segment = detail::attach_curve_start_to_circle(
                        segment, as_point(from_position), from_node.radius
                    );
                }
                if (edge.arrow_tip && index + 1 == edge.spline.size())
                {
                    segment = detail::attach_curve_to_circle(
                        segment, as_point(to_position), to_node.radius
                    );
                }
                draw_list->AddBezierCubic(
                    as_vector(segment.start),
                    as_vector(segment.first_control),
                    as_vector(segment.second_control),
                    as_vector(segment.end),
                    line_color(),
                    2.0f,
                    segment_count
                );
                final_segment = segment;
            }

            if (!edge.arrow_tip || !final_segment.has_value())
            {
                return;
            }

            const ImVec2 last_control_point = as_vector(final_segment->second_control);
            const ImVec2 arrow_tip = as_vector(final_segment->end);
            const ImVec2 arrow_direction = difference(arrow_tip, last_control_point);
            draw_arrow_head(draw_list, arrow_tip, arrow_direction, line_color());
        }

        // Evaluates a cubic Bezier curve at its midpoint.
        [[nodiscard]] ImVec2 cubic_midpoint(
            const ImVec2& start,
            const ImVec2& control_one,
            const ImVec2& control_two,
            const ImVec2& end
        )
        {
            return ImVec2(
                (start.x + 3.0f * control_one.x + 3.0f * control_two.x + end.x) / 8.0f,
                (start.y + 3.0f * control_one.y + 3.0f * control_two.y + end.y) / 8.0f
            );
        }

        // Preserves the original route's side while bounding its bend for moved endpoints.
        [[nodiscard]] float redrawn_edge_bend(
            const ::graph::Edge& edge,
            const ImVec2& original_from_position,
            const ImVec2& original_to_position,
            const CanvasTransform& transform
        )
        {
            if (edge.spline.empty())
            {
                return 0.0F;
            }

            const std::optional<ImVec2> direction =
                normalized(difference(original_to_position, original_from_position));
            if (!direction.has_value())
            {
                return 0.0F;
            }

            const ::graph::CubicBezier reference =
                transformed_segment(edge.spline[edge.spline.size() / 2], transform);
            const ImVec2 reference_midpoint = cubic_midpoint(
                as_vector(reference.start),
                as_vector(reference.first_control),
                as_vector(reference.second_control),
                as_vector(reference.end)
            );
            const ImVec2 centers_midpoint(
                (original_from_position.x + original_to_position.x) * 0.5F,
                (original_from_position.y + original_to_position.y) * 0.5F
            );
            const ImVec2 normal(-direction->y, direction->x);
            const ImVec2 offset = difference(reference_midpoint, centers_midpoint);
            return offset.x * normal.x + offset.y * normal.y;
        }

        // Draws a freshly routed cubic edge after either endpoint was moved.
        void draw_redrawn_edge(
            ImDrawList* draw_list,
            const detail::RedrawnEdgeGeometry& geometry,
            const bool draw_arrow
        )
        {
            const int segment_count =
                std::max(1, static_cast<int>(ImGui::GetIO().DisplayFramebufferScale.x * 20.0F));
            draw_list->AddBezierCubic(
                as_vector(geometry.curve.start),
                as_vector(geometry.curve.first_control),
                as_vector(geometry.curve.second_control),
                as_vector(geometry.curve.end),
                line_color(),
                2.0F,
                segment_count
            );
            if (draw_arrow)
            {
                draw_arrow_head(
                    draw_list,
                    as_vector(geometry.curve.end),
                    as_vector(geometry.arrow_direction),
                    line_color()
                );
            }
        }

        // Derives a readable label position beside the middle spline segment.
        [[nodiscard]] std::optional<ImVec2>
        curved_label_position(const ::graph::Edge& edge, const CanvasTransform& transform)
        {
            if (edge.spline.empty())
            {
                return std::nullopt;
            }

            const std::size_t segment_index = edge.spline.size() / 2;
            const ::graph::CubicBezier segment =
                transformed_segment(edge.spline[segment_index], transform);
            const ImVec2 start = as_vector(segment.start);
            const ImVec2 control_one = as_vector(segment.first_control);
            const ImVec2 control_two = as_vector(segment.second_control);
            const ImVec2 end = as_vector(segment.end);
            const ImVec2 midpoint = cubic_midpoint(start, control_one, control_two, end);
            const std::optional<ImVec2> tangent = normalized(ImVec2(
                -start.x - control_one.x + control_two.x + end.x,
                -start.y - control_one.y + control_two.y + end.y
            ));

            if (!tangent.has_value())
            {
                return translated(midpoint, ImVec2(1.0f, -1.0f), 12.0f);
            }

            return translated(midpoint, ImVec2(-tangent->y, tangent->x), 18.0f);
        }

        // Chooses and adjusts the best available label anchor for an edge.
        [[nodiscard]] std::optional<ImVec2> edge_label_position(
            const ::graph::Edge& edge,
            const ::graph::Node& from_node,
            const ::graph::Node& to_node,
            const ImVec2& from_position,
            const ImVec2& to_position,
            const CanvasTransform& transform
        )
        {
            if (edge.label_position)
            {
                ImVec2 position = transform(*edge.label_position);
                const float half_text_width = ImGui::CalcTextSize(edge.label.c_str()).x * 0.5f;
                if (edge.label_alignment == ::graph::LabelAlignment::Left)
                {
                    position.x -= half_text_width;
                }
                else if (edge.label_alignment == ::graph::LabelAlignment::Right)
                {
                    position.x += half_text_width;
                }
                return position;
            }

            if (!edge.spline.empty())
            {
                return curved_label_position(edge, transform);
            }

            const std::optional<LineSegment> line =
                make_line_segment(from_node, to_node, from_position, to_position);
            if (!line.has_value())
            {
                return std::nullopt;
            }

            const ImVec2 midpoint(
                (line->start.x + line->end.x) * 0.5f, (line->start.y + line->end.y) * 0.5f
            );
            return translated(midpoint, ImVec2(-line->direction.y, line->direction.x), 22.0f);
        }

        // Resolves endpoints and draws one straight, curved, or self-loop edge.
        void draw_edge(
            ImDrawList* draw_list,
            const ::graph::Layout& layout,
            const ::graph::Edge& edge,
            const NodeIndex& node_indices,
            const std::vector<ImVec2>& screen_positions,
            const GraphCanvasState& state,
            const CanvasTransform& transform
        )
        {
            const std::optional<std::size_t> from_index =
                find_node_index(node_indices, edge.source);
            const std::optional<std::size_t> to_index = find_node_index(node_indices, edge.target);
            if (!from_index.has_value() || !to_index.has_value())
            {
                return;
            }

            const ::graph::Node& from_node = layout.nodes[*from_index];
            const ::graph::Node& to_node = layout.nodes[*to_index];
            const ImVec2& from_position = screen_positions[*from_index];
            const ImVec2& to_position = screen_positions[*to_index];

            if (edge.source == edge.target)
            {
                draw_self_loop(draw_list, from_position, from_node.radius, edge.label);
                return;
            }

            std::optional<detail::RedrawnEdgeGeometry> redrawn_geometry;
            const bool endpoint_moved = state.node_positions.contains(from_node.id) ||
                                        state.node_positions.contains(to_node.id);
            if (!edge.spline.empty() && endpoint_moved)
            {
                const ImVec2 original_from_position = transform(from_node.position);
                const ImVec2 original_to_position = transform(to_node.position);
                redrawn_geometry = detail::make_redrawn_edge_geometry(
                    as_point(from_position),
                    from_node.role == ::graph::NodeRole::StartMarker ? 0.0F : from_node.radius,
                    as_point(to_position),
                    to_node.role == ::graph::NodeRole::StartMarker ? 0.0F : to_node.radius,
                    redrawn_edge_bend(edge, original_from_position, original_to_position, transform)
                );
                if (redrawn_geometry.has_value())
                {
                    draw_redrawn_edge(draw_list, *redrawn_geometry, edge.arrow_tip.has_value());
                }
            }
            else if (!edge.spline.empty())
            {
                draw_curved_edge(
                    draw_list, edge, from_node, to_node, from_position, to_position, transform
                );
            }
            else
            {
                const std::optional<LineSegment> line =
                    make_line_segment(from_node, to_node, from_position, to_position);
                if (!line.has_value())
                {
                    return;
                }

                draw_list->AddLine(line->start, line->end, line_color(), 2.0f);
                draw_arrow_head(draw_list, line->end, line->direction, line_color());
            }

            if (from_node.role == ::graph::NodeRole::StartMarker || edge.label.empty())
            {
                return;
            }

            const std::optional<ImVec2> label_position =
                redrawn_geometry.has_value()
                    ? std::optional<ImVec2>{as_vector(redrawn_geometry->label_position)}
                    : edge_label_position(
                          edge, from_node, to_node, from_position, to_position, transform
                      );
            if (label_position.has_value())
            {
                draw_label(draw_list, edge.label, *label_position);
            }
        }

        // Updates node positions directly while the user drags them.
        [[nodiscard]] NodeInteraction interact_with_nodes(
            const ::graph::Layout& layout,
            GraphCanvasState& state,
            const CanvasTransform& transform,
            const ImVec2& canvas_position,
            const ImVec2& canvas_size,
            std::vector<ImVec2>& screen_positions
        )
        {
            NodeInteraction interaction;
            interaction.visual_states.resize(layout.nodes.size());
            const ImVec2 saved_cursor = ImGui::GetCursorScreenPos();

            for (std::size_t index = 0; index < layout.nodes.size(); ++index)
            {
                const ::graph::Node& node = layout.nodes[index];
                if (node.role == ::graph::NodeRole::StartMarker)
                {
                    continue;
                }

                const float diameter = node.radius * 2.0F;
                ImGui::SetCursorScreenPos(ImVec2(
                    screen_positions[index].x - node.radius, screen_positions[index].y - node.radius
                ));
                ImGui::PushID(node.id.c_str());
                ImGui::InvisibleButton("##drag_handle", ImVec2(diameter, diameter));

                NodeVisualState& visual_state = interaction.visual_states[index];
                visual_state.hovered = ImGui::IsItemHovered();
                visual_state.active = ImGui::IsItemActive();
                interaction.any_active = interaction.any_active || visual_state.active;
                if (visual_state.hovered || visual_state.active)
                {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                }

                if (visual_state.active && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0F))
                {
                    const ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
                    ImVec2 next_position(
                        screen_positions[index].x + mouse_delta.x,
                        screen_positions[index].y + mouse_delta.y
                    );
                    const float minimum_x = canvas_position.x + node.radius;
                    const float minimum_y = canvas_position.y + node.radius;
                    const float maximum_x =
                        std::max(minimum_x, canvas_position.x + canvas_size.x - node.radius);
                    const float maximum_y =
                        std::max(minimum_y, canvas_position.y + canvas_size.y - node.radius);
                    next_position.x = std::clamp(next_position.x, minimum_x, maximum_x);
                    next_position.y = std::clamp(next_position.y, minimum_y, maximum_y);

                    screen_positions[index] = next_position;
                    set_node_position(state, node, transform.to_layout(next_position));
                }
                ImGui::PopID();
            }

            ImGui::SetCursorScreenPos(saved_cursor);
            return interaction;
        }

        // Draws one regular automaton state using its interaction colors.
        void draw_node(
            ImDrawList* draw_list,
            const ::graph::Node& node,
            const ImVec2& position,
            const NodeVisualState visual_state
        )
        {
            if (node.role == ::graph::NodeRole::StartMarker)
            {
                return;
            }

            const ImU32 fill_color = visual_state.active    ? ActiveNodeColor
                                     : visual_state.hovered ? HoveredNodeColor
                                                            : NodeColor;
            draw_list->AddCircleFilled(position, node.radius, fill_color, 32);
            draw_list->AddCircle(position, node.radius, line_color(), 32, 2.0f);

            if (node.is_final)
            {
                draw_list->AddCircle(position, node.radius - 5.0f, line_color(), 32, 2.0f);
            }

            draw_label(draw_list, node.label, position);
        }
    }

    void render_automaton_graph(
        const ::graph::Layout& layout,
        GraphCanvasState& state,
        const ImVec2& size,
        const char* child_id
    )
    {
        if (!ImGui::BeginChild(
                child_id,
                size,
                true,
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
            ))
        {
            ImGui::EndChild();
            return;
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImVec2 canvas_position = ImGui::GetCursorScreenPos();
        const ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        draw_list->AddRectFilled(
            canvas_position,
            ImVec2(canvas_position.x + canvas_size.x, canvas_position.y + canvas_size.y),
            canvas_color()
        );

        if (layout.nodes.empty())
        {
            ImGui::TextUnformatted("No automaton to display.");
            ImGui::Dummy(canvas_size);
            ImGui::EndChild();
            return;
        }

        capture_graph_wheel(canvas_position, canvas_size);
        apply_mouse_wheel_zoom(layout, state, canvas_position, canvas_size);
        CanvasTransform transform(layout, state, canvas_position, canvas_size);
        clamp_view_to_layout(layout, state, canvas_size, transform.scale);
        transform = CanvasTransform(layout, state, canvas_position, canvas_size);
        NodeIndex node_indices;
        node_indices.reserve(layout.nodes.size());
        std::vector<ImVec2> screen_positions;
        screen_positions.reserve(layout.nodes.size());

        for (std::size_t index = 0; index < layout.nodes.size(); ++index)
        {
            const ::graph::Node& node = layout.nodes[index];
            node_indices.emplace(node.id, index);
            screen_positions.push_back(transform(node_position(state, node)));
        }

        const NodeInteraction interaction = interact_with_nodes(
            layout, state, transform, canvas_position, canvas_size, screen_positions
        );

        if (apply_background_pan(state, transform.scale, interaction.any_active))
        {
            clamp_view_to_layout(layout, state, canvas_size, transform.scale);
            transform = CanvasTransform(layout, state, canvas_position, canvas_size);
            for (std::size_t index = 0; index < layout.nodes.size(); ++index)
            {
                screen_positions[index] = transform(node_position(state, layout.nodes[index]));
            }
        }

        for (const ::graph::Edge& edge : layout.edges)
        {
            draw_edge(draw_list, layout, edge, node_indices, screen_positions, state, transform);
        }

        for (std::size_t index = 0; index < layout.nodes.size(); ++index)
        {
            draw_node(
                draw_list,
                layout.nodes[index],
                screen_positions[index],
                interaction.visual_states[index]
            );
        }

        draw_minimap(draw_list, layout, state, transform, canvas_position, canvas_size);

        ImGui::SetCursorScreenPos(canvas_position);
        ImGui::Dummy(canvas_size);
        ImGui::EndChild();
    }
}
