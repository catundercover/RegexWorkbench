// Defines graph-canvas state and rendering-independent geometry helpers.
#pragma once

#include "graph/model/Layout.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>

namespace ui::graph
{
    // Smallest supported graph zoom factor.
    inline constexpr float MinimumGraphZoom = 0.4F;
    // Largest supported graph zoom factor.
    inline constexpr float MaximumGraphZoom = 4.0F;
    // Multiplicative zoom factor applied per input step.
    inline constexpr float GraphZoomStep = 1.2F;

    // User-adjusted node positions for one rendered automaton.
    struct GraphCanvasState
    {
        std::unordered_map<std::string, ::graph::Point> node_positions;
        bool background_drag_active = false;
        float zoom = 1.0F;
        ::graph::Point view_offset;
    };

    // Returns a node's user override or its original layout position.
    [[nodiscard]] ::graph::Point
    node_position(const GraphCanvasState& state, const ::graph::Node& node);

    // Records a user-adjusted position for a graph node.
    void
    set_node_position(GraphCanvasState& state, const ::graph::Node& node, ::graph::Point position);

    // Applies continuous zoom steps and clamps the result to the supported range.
    void zoom_graph(GraphCanvasState& state, float steps);

    // Moves the viewport so graph content follows a screen-space background drag.
    void pan_graph(GraphCanvasState& state, ::graph::Point screen_delta, float scale);

    // Clamps the viewport offset to the supplied horizontal and vertical limits.
    void clamp_graph_view(GraphCanvasState& state, ::graph::Point maximum_offset);

    // Restores Graphviz node positions and the original fitted viewport.
    void reset_graph(GraphCanvasState& state);

    // Fits the current node arrangement without discarding user-adjusted positions.
    void fit_graph_view(GraphCanvasState& state);

    // Returns whether zoom, viewport, and node positions retain their defaults.
    [[nodiscard]] bool is_initial_graph_view(const GraphCanvasState& state);

    namespace detail
    {
        // Geometry required to render a stable self-loop around one node.
        struct SelfLoopGeometry
        {
            ::graph::CubicBezier curve;
            ::graph::Point arrow_direction;
            ::graph::Point label_position;
        };

        // Geometry for an edge rebuilt from the current positions of two nodes.
        struct RedrawnEdgeGeometry
        {
            ::graph::CubicBezier curve;
            ::graph::Point arrow_direction;
            ::graph::Point label_position;
        };

        // Builds an open loop whose endpoints lie on the node circumference.
        [[nodiscard]] SelfLoopGeometry
        make_self_loop_geometry(::graph::Point center, float node_radius);

        // Builds a node-clipped cubic edge and label anchor for moved endpoints.
        [[nodiscard]] std::optional<RedrawnEdgeGeometry> make_redrawn_edge_geometry(
            ::graph::Point source_center,
            float source_radius,
            ::graph::Point target_center,
            float target_radius,
            float bend
        );

        // Projects a reference point onto the nearest side of a circle.
        [[nodiscard]] std::optional<::graph::Point>
        closest_point_on_circle(::graph::Point center, float radius, ::graph::Point reference);

        // Reattaches an incoming curve to the nearest side of a moved target circle.
        [[nodiscard]] ::graph::CubicBezier
        attach_curve_to_circle(::graph::CubicBezier curve, ::graph::Point center, float radius);

        // Reattaches an outgoing curve to the nearest side of a moved source circle.
        [[nodiscard]] ::graph::CubicBezier attach_curve_start_to_circle(
            ::graph::CubicBezier curve, ::graph::Point center, float radius
        );

    }
}
