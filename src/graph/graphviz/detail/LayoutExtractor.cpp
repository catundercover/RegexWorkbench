// Implements conversion from Graphviz objects to UI graph geometry.
#include "graph/graphviz/detail/LayoutExtractor.hpp"

#include "graph/graphviz/detail/AttributeParser.hpp"

#include <algorithm>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

extern "C"
{
#include <graphviz/cgraph.h>
}

namespace graph::graphviz::detail
{
    namespace
    {
        // Display radius assigned to regular automaton states.
        constexpr float NodeRadius = 24.0F;
        // Extra space retained around extracted graph geometry.
        constexpr float LayoutPadding = 50.0F;

        // Returns a borrowed Graphviz attribute value or an empty view.
        [[nodiscard]] std::string_view attribute(void* object, const char* name)
        {
            const char* value = agget(object, const_cast<char*>(name));
            return value != nullptr ? std::string_view(value) : std::string_view{};
        }

        // Builds a consistent diagnostic for malformed Graphviz output.
        [[nodiscard]] LayoutResult invalid_output(const std::string_view detail)
        {
            return LayoutError{
                LayoutErrorCode::InvalidOutput,
                "Graphviz produced invalid layout data: " + std::string(detail)
            };
        }

        // Detects Graphviz shapes that represent accepting automaton states.
        [[nodiscard]] bool is_final_node(Agnode_t* node)
        {
            const std::string_view shape = attribute(node, "shape");
            if (shape == "doublecircle" || shape == "doubleoctagon")
            {
                return true;
            }
            return attribute(node, "peripheries") == "2";
        }

        // Maps a Graphviz point node to the synthetic start-marker role.
        [[nodiscard]] NodeRole node_role(Agnode_t* node)
        {
            return attribute(node, "shape") == "point" ? NodeRole::StartMarker : NodeRole::State;
        }

        // Expands derived layout bounds to include one point.
        void expand_bounds(Bounds& bounds, const Point point)
        {
            bounds.minimum.x = std::min(bounds.minimum.x, point.x);
            bounds.minimum.y = std::min(bounds.minimum.y, point.y);
            bounds.maximum.x = std::max(bounds.maximum.x, point.x);
            bounds.maximum.y = std::max(bounds.maximum.y, point.y);
        }

        /// Adds consistent rendering space around extracted bounds.
        void add_padding(Bounds& bounds)
        {
            bounds.minimum.x -= LayoutPadding;
            bounds.minimum.y -= LayoutPadding;
            bounds.maximum.x += LayoutPadding;
            bounds.maximum.y += LayoutPadding;
        }
    }

    LayoutResult extract_layout(Agraph_s* graph)
    {
        if (graph == nullptr)
        {
            return invalid_output("the graph is missing.");
        }

        Layout layout;
        const std::string_view raw_bounds = attribute(graph, "bb");
        const std::optional<Bounds> parsed_bounds = parse_bounds(raw_bounds);
        if (!raw_bounds.empty() && !parsed_bounds)
        {
            return invalid_output("the graph bounding box is malformed.");
        }

        const bool derive_bounds = !parsed_bounds.has_value();
        if (parsed_bounds)
        {
            layout.bounds = *parsed_bounds;
        }
        else
        {
            layout.bounds = Bounds{
                Point{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()},
                Point{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()}
            };
        }

        for (Agnode_t* node = agfstnode(graph); node != nullptr; node = agnxtnode(graph, node))
        {
            const char* raw_name = agnameof(node);
            if (raw_name == nullptr || *raw_name == '\0')
            {
                return invalid_output("a node has no identifier.");
            }

            const std::optional<Point> position = parse_point(attribute(node, "pos"));
            if (!position)
            {
                return invalid_output(
                    "node '" + std::string(raw_name) + "' has no valid position."
                );
            }

            const std::string id(raw_name);
            const std::string_view raw_label = attribute(node, "label");
            const NodeRole role = node_role(node);
            layout.nodes.push_back(
                Node{
                    id,
                    raw_label.empty() || raw_label == "\\N" ? id : std::string(raw_label),
                    *position,
                    role == NodeRole::StartMarker ? 0.0F : NodeRadius,
                    is_final_node(node),
                    role
                }
            );

            if (derive_bounds)
            {
                expand_bounds(layout.bounds, *position);
            }
        }

        if (layout.nodes.empty())
        {
            return invalid_output("no positioned nodes were produced.");
        }

        for (Agnode_t* node = agfstnode(graph); node != nullptr; node = agnxtnode(graph, node))
        {
            for (Agedge_t* edge = agfstout(graph, node); edge != nullptr;
                 edge = agnxtout(graph, edge))
            {
                Agnode_t* tail = agtail(edge);
                Agnode_t* head = aghead(edge);
                const char* raw_source = tail != nullptr ? agnameof(tail) : nullptr;
                const char* raw_target = head != nullptr ? agnameof(head) : nullptr;
                if (raw_source == nullptr || *raw_source == '\0' || raw_target == nullptr ||
                    *raw_target == '\0')
                {
                    return invalid_output("an edge has an invalid endpoint.");
                }

                Edge edge_layout;
                edge_layout.source = raw_source;
                edge_layout.target = raw_target;
                edge_layout.label = attribute(edge, "label");

                const std::string_view raw_spline = attribute(edge, "pos");
                if (!raw_spline.empty())
                {
                    std::optional<ParsedSpline> spline = parse_spline(raw_spline);
                    if (!spline)
                    {
                        return invalid_output(
                            "edge '" + edge_layout.source + "' -> '" + edge_layout.target +
                            "' has a malformed spline."
                        );
                    }
                    edge_layout.spline = std::move(spline->segments);
                    edge_layout.arrow_tip = spline->arrow_tip;
                }

                const std::string_view raw_label_position = attribute(edge, "lp");
                if (!raw_label_position.empty())
                {
                    edge_layout.label_position = parse_point(raw_label_position);
                    if (!edge_layout.label_position)
                    {
                        return invalid_output(
                            "edge '" + edge_layout.source + "' -> '" + edge_layout.target +
                            "' has a malformed label position."
                        );
                    }
                }
                edge_layout.label_alignment = parse_label_alignment(attribute(edge, "labeljust"));
                layout.edges.push_back(std::move(edge_layout));
            }
        }

        add_padding(layout.bounds);
        return layout;
    }
}
