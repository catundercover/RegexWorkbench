// Defines the rendering-independent graph layout model.
#pragma once

#include "graph/model/Geometry.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace graph
{
    // Distinguishes automaton states from synthetic start-arrow markers.
    enum class NodeRole : std::uint8_t
    {
        State,
        StartMarker
    };

    // Describes horizontal alignment around an edge-label anchor.
    enum class LabelAlignment : std::uint8_t
    {
        Center,
        Left,
        Right
    };

    // Positioned automaton node ready for UI rendering.
    struct Node
    {
        std::string id;
        std::string label;
        Point position;
        float radius = 24.0F;
        bool is_final = false;
        NodeRole role = NodeRole::State;

        // Compares all node attributes exactly.
        bool operator==(const Node&) const = default;
    };

    // Positioned, labeled connection between two graph nodes.
    struct Edge
    {
        std::string source;
        std::string target;
        std::string label;
        std::vector<CubicBezier> spline;
        std::optional<Point> arrow_tip;
        std::optional<Point> label_position;
        LabelAlignment label_alignment = LabelAlignment::Center;

        // Compares all edge attributes exactly.
        bool operator==(const Edge&) const = default;
    };

    // Complete graph geometry and its coordinate bounds.
    struct Layout
    {
        std::vector<Node> nodes;
        std::vector<Edge> edges;
        Bounds bounds;

        // Compares nodes, edges, and bounds exactly.
        bool operator==(const Layout&) const = default;
    };
}
