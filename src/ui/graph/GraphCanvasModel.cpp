// Implements graph viewport state and interactive edge geometry.
#include "ui/graph/GraphCanvasModel.hpp"

#include <algorithm>
#include <cmath>

namespace ui::graph
{
    namespace
    {
        // Returns the Euclidean distance between two graph points.
        [[nodiscard]] float distance(const ::graph::Point first, const ::graph::Point second)
        {
            return std::hypot(first.x - second.x, first.y - second.y);
        }
    }

    ::graph::Point node_position(const GraphCanvasState& state, const ::graph::Node& node)
    {
        const auto position = state.node_positions.find(node.id);
        return position == state.node_positions.end() ? node.position : position->second;
    }

    void set_node_position(
        GraphCanvasState& state, const ::graph::Node& node, const ::graph::Point position
    )
    {
        state.node_positions.insert_or_assign(node.id, position);
    }

    void zoom_graph(GraphCanvasState& state, const float steps)
    {
        if (!std::isfinite(state.zoom) || !std::isfinite(steps))
        {
            reset_graph(state);
            return;
        }

        state.zoom = std::clamp(
            state.zoom * std::pow(GraphZoomStep, steps), MinimumGraphZoom, MaximumGraphZoom
        );
    }

    void pan_graph(GraphCanvasState& state, const ::graph::Point screen_delta, const float scale)
    {
        if (!std::isfinite(screen_delta.x) || !std::isfinite(screen_delta.y) ||
            !std::isfinite(scale) || scale <= 0.0F)
        {
            return;
        }

        state.view_offset.x -= screen_delta.x / scale;
        state.view_offset.y += screen_delta.y / scale;
    }

    void clamp_graph_view(GraphCanvasState& state, const ::graph::Point maximum_offset)
    {
        if (!std::isfinite(state.view_offset.x) || !std::isfinite(state.view_offset.y))
        {
            state.view_offset = {};
        }

        const float maximum_x =
            std::isfinite(maximum_offset.x) ? std::max(0.0F, maximum_offset.x) : 0.0F;
        const float maximum_y =
            std::isfinite(maximum_offset.y) ? std::max(0.0F, maximum_offset.y) : 0.0F;
        state.view_offset.x = std::clamp(state.view_offset.x, -maximum_x, maximum_x);
        state.view_offset.y = std::clamp(state.view_offset.y, -maximum_y, maximum_y);
    }

    void reset_graph(GraphCanvasState& state)
    {
        state = GraphCanvasState{};
    }

    void fit_graph_view(GraphCanvasState& state)
    {
        state.background_drag_active = false;
        state.zoom = 1.0F;
        state.view_offset = {};
    }

    bool is_initial_graph_view(const GraphCanvasState& state)
    {
        return state.node_positions.empty() && state.zoom == 1.0F &&
               state.view_offset == ::graph::Point{};
    }

    namespace detail
    {
        std::optional<::graph::Point> closest_point_on_circle(
            const ::graph::Point center, const float radius, const ::graph::Point reference
        )
        {
            constexpr float MinimumDirectionLength = 0.001F;
            const float horizontal = reference.x - center.x;
            const float vertical = reference.y - center.y;
            const float direction_length = distance(reference, center);
            if (!std::isfinite(radius) || radius <= 0.0F ||
                direction_length <= MinimumDirectionLength)
            {
                return std::nullopt;
            }

            return ::graph::Point{
                center.x + horizontal / direction_length * radius,
                center.y + vertical / direction_length * radius
            };
        }

        ::graph::CubicBezier attach_curve_to_circle(
            ::graph::CubicBezier curve, const ::graph::Point center, const float radius
        )
        {
            // Fall back toward the curve start when Graphviz placed controls inside the node.
            ::graph::Point attachment_reference = curve.second_control;
            if (distance(attachment_reference, center) <= radius)
            {
                attachment_reference = curve.first_control;
                if (distance(attachment_reference, center) <= radius)
                {
                    attachment_reference = curve.start;
                }
                curve.second_control = attachment_reference;
            }

            const std::optional<::graph::Point> attachment =
                closest_point_on_circle(center, radius, attachment_reference);
            if (attachment.has_value())
            {
                curve.end = *attachment;
            }
            return curve;
        }

        ::graph::CubicBezier attach_curve_start_to_circle(
            ::graph::CubicBezier curve, const ::graph::Point center, const float radius
        )
        {
            // Fall back toward the curve end when Graphviz placed controls inside the node.
            ::graph::Point attachment_reference = curve.first_control;
            if (distance(attachment_reference, center) <= radius)
            {
                attachment_reference = curve.second_control;
                if (distance(attachment_reference, center) <= radius)
                {
                    attachment_reference = curve.end;
                }
                curve.first_control = attachment_reference;
            }

            const std::optional<::graph::Point> attachment =
                closest_point_on_circle(center, radius, attachment_reference);
            if (attachment.has_value())
            {
                curve.start = *attachment;
            }
            return curve;
        }

        SelfLoopGeometry
        make_self_loop_geometry(const ::graph::Point center, const float node_radius)
        {
            constexpr float EndpointHorizontalFactor = 0.72F;
            constexpr float MinimumControlHeight = 44.0F;
            constexpr float MinimumControlSpread = 32.0F;
            constexpr float LabelClearance = 16.0F;

            const float radius = std::max(node_radius, 1.0F);
            const float endpoint_x = radius * EndpointHorizontalFactor;
            const float endpoint_y =
                -std::sqrt(std::max(0.0F, radius * radius - endpoint_x * endpoint_x));
            const float control_height = std::max(MinimumControlHeight, radius * 2.0F);
            const float control_spread = std::max(MinimumControlSpread, radius * 1.5F);

            const ::graph::Point start{center.x - endpoint_x, center.y + endpoint_y};
            const ::graph::Point end{center.x + endpoint_x, center.y + endpoint_y};
            const ::graph::Point first_control{center.x - control_spread, start.y - control_height};
            const ::graph::Point second_control{center.x + control_spread, end.y - control_height};

            return SelfLoopGeometry{
                ::graph::CubicBezier{start, first_control, second_control, end},
                ::graph::Point{end.x - second_control.x, end.y - second_control.y},
                ::graph::Point{center.x, first_control.y - LabelClearance}
            };
        }

        std::optional<RedrawnEdgeGeometry> make_redrawn_edge_geometry(
            const ::graph::Point source_center,
            const float source_radius,
            const ::graph::Point target_center,
            const float target_radius,
            const float bend
        )
        {
            constexpr float MinimumDirectionLength = 0.001F;
            constexpr float MaximumRadiusFraction = 0.4F;
            constexpr float ControlFraction = 1.0F / 3.0F;
            constexpr float MaximumBend = 96.0F;
            constexpr float LabelClearance = 18.0F;

            const float center_distance = distance(source_center, target_center);
            if (!std::isfinite(center_distance) || center_distance <= MinimumDirectionLength)
            {
                return std::nullopt;
            }

            const ::graph::Point direction{
                (target_center.x - source_center.x) / center_distance,
                (target_center.y - source_center.y) / center_distance
            };
            const ::graph::Point normal{-direction.y, direction.x};
            const float usable_source_radius = std::min(
                std::max(std::isfinite(source_radius) ? source_radius : 0.0F, 0.0F),
                center_distance * MaximumRadiusFraction
            );
            const float usable_target_radius = std::min(
                std::max(std::isfinite(target_radius) ? target_radius : 0.0F, 0.0F),
                center_distance * MaximumRadiusFraction
            );
            const ::graph::Point start{
                source_center.x + direction.x * usable_source_radius,
                source_center.y + direction.y * usable_source_radius
            };
            const ::graph::Point end{
                target_center.x - direction.x * usable_target_radius,
                target_center.y - direction.y * usable_target_radius
            };
            const float chord_length = distance(start, end);
            const float maximum_bend = std::min(MaximumBend, center_distance * 0.35F);
            const float curve_bend =
                std::clamp(std::isfinite(bend) ? bend : 0.0F, -maximum_bend, maximum_bend);
            const float handle_length = chord_length * ControlFraction;

            const ::graph::Point first_control{
                start.x + direction.x * handle_length + normal.x * curve_bend,
                start.y + direction.y * handle_length + normal.y * curve_bend
            };
            const ::graph::Point second_control{
                end.x - direction.x * handle_length + normal.x * curve_bend,
                end.y - direction.y * handle_length + normal.y * curve_bend
            };
            const ::graph::CubicBezier curve{start, first_control, second_control, end};
            const ::graph::Point midpoint{
                (start.x + 3.0F * first_control.x + 3.0F * second_control.x + end.x) / 8.0F,
                (start.y + 3.0F * first_control.y + 3.0F * second_control.y + end.y) / 8.0F
            };
            const float label_side = curve_bend < 0.0F ? -1.0F : 1.0F;

            return RedrawnEdgeGeometry{
                curve,
                ::graph::Point{end.x - second_control.x, end.y - second_control.y},
                ::graph::Point{
                    midpoint.x + normal.x * LabelClearance * label_side,
                    midpoint.y + normal.y * LabelClearance * label_side
                }
            };
        }

    }
}
