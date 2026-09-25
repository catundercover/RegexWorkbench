// Verifies graph canvas state and spline deformation geometry.
#include "ui/graph/GraphCanvasModel.hpp"

#include "support/TestSupport.hpp"
#include "ui/graph/AutomatonText.hpp"

#include <cmath>
#include <limits>
#include <string>

namespace
{
    // Absolute tolerance used for floating-point geometry checks.
    constexpr float Tolerance = 0.001F;

    // Returns whether two scalar coordinates are equal within the test tolerance.
    [[nodiscard]] bool approximately_equal(const float left, const float right)
    {
        return std::abs(left - right) <= Tolerance;
    }

    // Returns whether both coordinates of two points are approximately equal.
    [[nodiscard]] bool approximately_equal(const graph::Point left, const graph::Point right)
    {
        return approximately_equal(left.x, right.x) && approximately_equal(left.y, right.y);
    }

    // Returns the Euclidean distance between two test points.
    [[nodiscard]] float distance(const graph::Point left, const graph::Point right)
    {
        const float x = left.x - right.x;
        const float y = left.y - right.y;
        return std::sqrt(x * x + y * y);
    }

    // Checks fallback and overridden node positions.
    bool test_canvas_state()
    {
        const graph::Node node{"node0", "0", {10.0F, 20.0F}};
        ui::graph::GraphCanvasState state;
        if (!test_support::check(
                ui::graph::node_position(state, node) == node.position,
                "Canvas state did not fall back to the Graphviz position."
            ))
        {
            return false;
        }

        constexpr graph::Point Moved{35.0F, 42.0F};
        ui::graph::set_node_position(state, node, Moved);
        return test_support::check(
            ui::graph::node_position(state, node) == Moved,
            "Canvas state did not preserve the moved node position."
        );
    }

    // Checks the formal text representation derived from graph topology.
    bool test_automaton_text()
    {
        graph::Layout layout;
        layout.nodes.push_back(
            graph::Node{"start", "", {}, 1.0F, false, graph::NodeRole::StartMarker}
        );
        layout.nodes.push_back(graph::Node{"q0", "0", {}, 24.0F, false});
        layout.nodes.push_back(graph::Node{"q1", "1", {}, 24.0F, true});
        graph::Edge start_edge;
        start_edge.source = "start";
        start_edge.target = "q0";
        layout.edges.push_back(start_edge);
        graph::Edge symbols_edge;
        symbols_edge.source = "q0";
        symbols_edge.target = "q1";
        symbols_edge.label = "a,b";
        layout.edges.push_back(symbols_edge);
        graph::Edge epsilon_edge;
        epsilon_edge.source = "q0";
        epsilon_edge.target = "q0";
        epsilon_edge.label = "ε";
        layout.edges.push_back(epsilon_edge);

        const std::string text = ui::graph::format_automaton_text(layout);
        return test_support::check(
                   text.find("A = (Q, Σ, δ, q₀, F)") != std::string::npos,
                   "The formal automaton tuple is missing."
               ) &&
               test_support::check(
                   text.find("Q = {0, 1}") != std::string::npos &&
                       text.find("Σ = {a, b}") != std::string::npos,
                   "States or alphabet were formatted incorrectly."
               ) &&
               test_support::check(
                   text.find("q₀ = 0") != std::string::npos &&
                       text.find("F = {1}") != std::string::npos,
                   "Initial or final states were formatted incorrectly."
               ) &&
               test_support::check(
                   text.find("δ(0, a) = {1}") != std::string::npos &&
                       text.find("δ(0, b) = {1}") != std::string::npos &&
                       text.find("δ(0, ε) = {0}") != std::string::npos,
                   "The transition relation was formatted incorrectly."
               );
    }

    // Checks zoom bounds and viewport reset behavior.
    bool test_canvas_view_controls()
    {
        const graph::Node node{"node0", "0", {10.0F, 20.0F}};
        ui::graph::GraphCanvasState state;
        if (!test_support::check(
                ui::graph::is_initial_graph_view(state),
                "A new canvas did not start in its initial view."
            ))
        {
            return false;
        }

        ui::graph::zoom_graph(state, 1.0F);
        if (!test_support::check(
                approximately_equal(state.zoom, ui::graph::GraphZoomStep),
                "A zoom step did not use the documented scale factor."
            ) ||
            !test_support::check(
                !ui::graph::is_initial_graph_view(state),
                "A zoomed canvas was reported as its initial view."
            ))
        {
            return false;
        }

        ui::graph::zoom_graph(state, 100.0F);
        if (!test_support::check(
                approximately_equal(state.zoom, ui::graph::MaximumGraphZoom),
                "Zoom was not clamped to its maximum."
            ))
        {
            return false;
        }

        ui::graph::zoom_graph(state, -100.0F);
        if (!test_support::check(
                approximately_equal(state.zoom, ui::graph::MinimumGraphZoom),
                "Zoom was not clamped to its minimum."
            ))
        {
            return false;
        }

        ui::graph::pan_graph(state, {120.0F, -60.0F}, 2.0F);
        if (!test_support::check(
                approximately_equal(state.view_offset, graph::Point{-60.0F, -30.0F}),
                "Background dragging did not move the viewport in layout coordinates."
            ))
        {
            return false;
        }
        ui::graph::clamp_graph_view(state, {25.0F, 10.0F});
        if (!test_support::check(
                approximately_equal(state.view_offset, graph::Point{-25.0F, -10.0F}),
                "Viewport navigation was not clamped to the graph area."
            ))
        {
            return false;
        }

        ui::graph::set_node_position(state, node, {30.0F, 40.0F});
        ui::graph::zoom_graph(state, 2.0F);
        ui::graph::pan_graph(state, {30.0F, 20.0F}, 2.0F);
        ui::graph::fit_graph_view(state);
        if (!test_support::check(
                ui::graph::node_position(state, node) == graph::Point{30.0F, 40.0F} &&
                    state.zoom == 1.0F && state.view_offset == graph::Point{},
                "Fit view did not preserve moved nodes while resetting the viewport."
            ))
        {
            return false;
        }

        ui::graph::set_node_position(state, node, {30.0F, 40.0F});
        ui::graph::reset_graph(state);
        if (!test_support::check(
                ui::graph::is_initial_graph_view(state),
                "Reset did not restore the complete initial graph view."
            ))
        {
            return false;
        }

        ui::graph::set_node_position(state, node, {30.0F, 40.0F});
        ui::graph::zoom_graph(state, std::numeric_limits<float>::quiet_NaN());
        return test_support::check(
            ui::graph::is_initial_graph_view(state),
            "A non-finite zoom input did not repair the canvas state."
        );
    }

    // Checks self-loop attachment, direction, and label placement.
    bool test_self_loop_geometry()
    {
        constexpr graph::Point Center{100.0F, 80.0F};
        constexpr float Radius = 24.0F;
        const ui::graph::detail::SelfLoopGeometry loop =
            ui::graph::detail::make_self_loop_geometry(Center, Radius);

        const graph::Point direction_to_center{
            Center.x - loop.curve.end.x, Center.y - loop.curve.end.y
        };
        const float inward_dot = loop.arrow_direction.x * direction_to_center.x +
                                 loop.arrow_direction.y * direction_to_center.y;

        return test_support::check(
                   approximately_equal(distance(loop.curve.start, Center), Radius),
                   "Self-loop start is not attached to the node boundary."
               ) &&
               test_support::check(
                   approximately_equal(distance(loop.curve.end, Center), Radius),
                   "Self-loop arrow tip is not attached to the node boundary."
               ) &&
               test_support::check(
                   loop.curve.start != loop.curve.end,
                   "Self-loop unexpectedly forms a closed circle."
               ) &&
               test_support::check(
                   inward_dot > 0.0F, "Self-loop arrow does not point back into the node."
               ) &&
               test_support::check(
                   loop.label_position.y < loop.curve.first_control.y,
                   "Self-loop label is not above the curve."
               );
    }

    // Checks incoming and outgoing curve attachment to a node circle.
    bool test_circle_attachment()
    {
        constexpr graph::Point Center{10.0F, 20.0F};
        constexpr float Radius = 5.0F;
        const std::optional<graph::Point> left =
            ui::graph::detail::closest_point_on_circle(Center, Radius, {-20.0F, 20.0F});
        const std::optional<graph::Point> diagonal =
            ui::graph::detail::closest_point_on_circle(Center, Radius, {13.0F, 24.0F});
        const graph::CubicBezier incoming{
            {0.0F, 20.0F}, {2.0F, 20.0F}, {4.0F, 20.0F}, {6.0F, 20.0F}
        };
        const graph::CubicBezier attached =
            ui::graph::detail::attach_curve_to_circle(incoming, Center, Radius);
        const graph::CubicBezier inside_control{
            {0.0F, 20.0F}, {2.0F, 20.0F}, {8.0F, 20.0F}, {9.0F, 20.0F}
        };
        const graph::CubicBezier repaired_control =
            ui::graph::detail::attach_curve_to_circle(inside_control, Center, Radius);
        const graph::CubicBezier outgoing{
            {14.0F, 20.0F}, {16.0F, 20.0F}, {18.0F, 20.0F}, {30.0F, 20.0F}
        };
        const graph::CubicBezier attached_start =
            ui::graph::detail::attach_curve_start_to_circle(outgoing, Center, Radius);

        return test_support::check(
                   left.has_value() && approximately_equal(*left, graph::Point{5.0F, 20.0F}),
                   "An incoming edge did not attach to the nearest side of its target."
               ) &&
               test_support::check(
                   diagonal.has_value() && approximately_equal(distance(*diagonal, Center), Radius),
                   "A diagonal edge attachment was not placed on the target circle."
               ) &&
               test_support::check(
                   !ui::graph::detail::closest_point_on_circle(Center, Radius, Center).has_value(),
                   "A directionless circle attachment should not be produced."
               ) &&
               test_support::check(
                   approximately_equal(attached.end, graph::Point{5.0F, 20.0F}),
                   "A moved target left its incoming curve detached from the circle."
               ) &&
               test_support::check(
                   repaired_control.second_control == inside_control.first_control &&
                       approximately_equal(repaired_control.end, graph::Point{5.0F, 20.0F}),
                   "An incoming control point inside the target produced a reversed arrow."
               ) &&
               test_support::check(
                   approximately_equal(attached_start.start, graph::Point{15.0F, 20.0F}),
                   "An outgoing curve did not start on the source circle."
               );
    }

    // Checks that moved endpoints produce fresh node-clipped cubic edge geometry.
    bool test_redrawn_edge_geometry()
    {
        constexpr graph::Point Source{0.0F, 0.0F};
        constexpr graph::Point HorizontalTarget{100.0F, 0.0F};
        constexpr graph::Point VerticalTarget{0.0F, 100.0F};
        const auto horizontal = ui::graph::detail::make_redrawn_edge_geometry(
            Source, 10.0F, HorizontalTarget, 20.0F, -30.0F
        );
        const auto vertical = ui::graph::detail::make_redrawn_edge_geometry(
            Source, 10.0F, VerticalTarget, 20.0F, -30.0F
        );

        if (!horizontal.has_value() || !vertical.has_value())
        {
            return test_support::check(
                false, "Moved endpoints did not produce redrawable edge geometry."
            );
        }

        const ui::graph::detail::RedrawnEdgeGeometry& horizontal_geometry = *horizontal;
        const ui::graph::detail::RedrawnEdgeGeometry& vertical_geometry = *vertical;
        return test_support::check(
                   approximately_equal(distance(horizontal_geometry.curve.start, Source), 10.0F) &&
                       approximately_equal(
                           distance(horizontal_geometry.curve.end, HorizontalTarget), 20.0F
                       ),
                   "A redrawn edge was not clipped to its node boundaries."
               ) &&
               test_support::check(
                   horizontal_geometry.curve.first_control.y < horizontal_geometry.curve.start.y &&
                       horizontal_geometry.label_position.y <
                           horizontal_geometry.curve.first_control.y,
                   "A redrawn edge did not preserve its requested bend and label side."
               ) &&
               test_support::check(
                   approximately_equal(distance(vertical_geometry.curve.start, Source), 10.0F) &&
                       approximately_equal(
                           distance(vertical_geometry.curve.end, VerticalTarget), 20.0F
                       ) &&
                       vertical_geometry.curve != horizontal_geometry.curve,
                   "Redrawn edge geometry did not respond to a moved target."
               ) &&
               test_support::check(
                   !ui::graph::detail::make_redrawn_edge_geometry(
                        Source, 10.0F, Source, 10.0F, 20.0F
                   )
                        .has_value(),
                   "Coincident endpoints unexpectedly produced unstable edge geometry."
               );
    }

}

// Runs all rendering-independent graph canvas checks.
int main()
{
    return test_canvas_state() && test_automaton_text() && test_canvas_view_controls() &&
                   test_self_loop_geometry() && test_circle_attachment() &&
                   test_redrawn_edge_geometry()
               ? 0
               : 1;
}
