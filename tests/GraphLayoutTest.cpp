// Verifies Graphviz layout extraction and parsing.
#include "graph/graphviz/GraphvizLayout.hpp"
#include "graph/graphviz/detail/AttributeParser.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <variant>

namespace
{
    // Reports a failed assertion and returns its condition.
    bool check(const bool condition, const std::string& message)
    {
        if (!condition)
        {
            std::cerr << message << '\n';
        }
        return condition;
    }

    // Checks valid and malformed Graphviz geometry attributes.
    bool test_attribute_parsing()
    {
        using namespace graph::graphviz::detail;

        const std::optional<graph::Point> point = parse_point("-12.5,3.25");
        if (!check(point == graph::Point{-12.5F, 3.25F}, "A valid point was not parsed.") ||
            !check(!parse_point("12"), "A point without a y coordinate was accepted.") ||
            !check(!parse_point("1,2,3"), "A point with extra coordinates was accepted.") ||
            !check(!parse_point("nan,1"), "A non-finite point was accepted."))
        {
            return false;
        }

        const std::optional<graph::Bounds> bounds = parse_bounds("-1,-2,30,40");
        if (!check(
                bounds == graph::Bounds{{-1.0F, -2.0F}, {30.0F, 40.0F}},
                "A valid bounding box was not parsed."
            ) ||
            !check(!parse_bounds("3,0,2,4"), "A reversed bounding box was accepted."))
        {
            return false;
        }

        const std::optional<ParsedSpline> spline =
            parse_spline("e,10,20 0,0 1,2 3,4 5,6 7,8 9,10 11,12");
        if (!spline)
        {
            return check(false, "A valid cubic spline was not parsed.");
        }

        const ParsedSpline& parsed_spline = *spline;
        if (!check(parsed_spline.segments.size() == 2, "The spline segment count is incorrect.") ||
            !check(
                parsed_spline.arrow_tip == graph::Point{10.0F, 20.0F}, "The arrow tip is wrong."
            ) ||
            !check(
                parsed_spline.segments[1].start == graph::Point{5.0F, 6.0F},
                "Adjacent spline segments do not share the expected endpoint."
            ) ||
            !check(!parse_spline("0,0 1,1 2,2"), "An incomplete cubic spline was accepted.") ||
            !check(!parse_spline("0,0 broken 1,1 2,2"), "A malformed spline was accepted."))
        {
            return false;
        }

        return check(
                   parse_label_alignment("l") == graph::LabelAlignment::Left,
                   "Left label alignment was not recognized."
               ) &&
               check(
                   parse_label_alignment("r") == graph::LabelAlignment::Right,
                   "Right label alignment was not recognized."
               ) &&
               check(
                   parse_label_alignment("") == graph::LabelAlignment::Center,
                   "Default label alignment is not centered."
               );
    }

    // Checks extraction of nodes, edges, labels, and bounds from Graphviz.
    bool test_graphviz_layout()
    {
        constexpr std::string_view Dot = R"dot(
            digraph Automaton {
                rankdir=LR;
                node [shape=circle];
                __start [shape=point, label="", width=0.1, height=0.1];
                node0 [label="0", shape=doublecircle];
                node1 [label="1"];
                __start -> node0;
                node0 -> node1 [label="a,b"];
                node1 -> node1 [label="c"];
            }
        )dot";

        graph::graphviz::LayoutResult result = graph::graphviz::compute_layout(Dot);
        const auto* error = std::get_if<graph::graphviz::LayoutError>(&result);
        if (!check(
                error == nullptr,
                error != nullptr ? "Graphviz layout failed: " + error->message
                                 : "Graphviz layout unexpectedly failed."
            ))
        {
            return false;
        }

        const auto* layout = std::get_if<graph::Layout>(&result);
        if (!check(layout != nullptr, "The Graphviz result contains no layout."))
        {
            return false;
        }

        const auto start = std::find_if(
            layout->nodes.begin(),
            layout->nodes.end(),
            [](const graph::Node& node) { return node.role == graph::NodeRole::StartMarker; }
        );
        const auto final = std::find_if(
            layout->nodes.begin(),
            layout->nodes.end(),
            [](const graph::Node& node) { return node.id == "node0"; }
        );
        const auto labeled_edge = std::find_if(
            layout->edges.begin(),
            layout->edges.end(),
            [](const graph::Edge& edge) { return edge.label == "a,b"; }
        );

        return check(layout->nodes.size() == 3, "The layout has an unexpected node count.") &&
               check(start != layout->nodes.end(), "The start marker role was not extracted.") &&
               check(start->radius == 0.0F, "The start marker has a state radius.") &&
               check(
                   final != layout->nodes.end() && final->is_final,
                   "The final state was not extracted."
               ) &&
               check(layout->edges.size() == 3, "The layout has an unexpected edge count.") &&
               check(labeled_edge != layout->edges.end(), "The labeled edge is missing.") &&
               check(!labeled_edge->spline.empty(), "The labeled edge has no cubic spline.") &&
               check(labeled_edge->arrow_tip.has_value(), "The labeled edge has no arrow tip.") &&
               check(
                   labeled_edge->label_position.has_value(),
                   "The labeled edge has no label position."
               ) &&
               check(
                   std::isfinite(layout->bounds.minimum.x) &&
                       std::isfinite(layout->bounds.minimum.y) &&
                       std::isfinite(layout->bounds.maximum.x) &&
                       std::isfinite(layout->bounds.maximum.y),
                   "The layout bounds are not finite."
               ) &&
               check(
                   layout->bounds.maximum.x > layout->bounds.minimum.x &&
                       layout->bounds.maximum.y > layout->bounds.minimum.y,
                   "The layout bounds are empty."
               );
    }

    // Checks controlled diagnostics for invalid layout inputs.
    bool test_graphviz_errors()
    {
        using graph::graphviz::LayoutError;
        using graph::graphviz::LayoutErrorCode;

        graph::graphviz::LayoutResult empty = graph::graphviz::compute_layout("");
        const auto* empty_error = std::get_if<LayoutError>(&empty);
        if (!check(
                empty_error != nullptr && empty_error->code == LayoutErrorCode::EmptyInput,
                "Empty DOT input did not return the typed empty-input error."
            ))
        {
            return false;
        }

        graph::graphviz::LayoutResult malformed = graph::graphviz::compute_layout("not dot");
        const auto* malformed_error = std::get_if<LayoutError>(&malformed);
        return check(
            malformed_error != nullptr && malformed_error->code == LayoutErrorCode::InvalidDot,
            "Malformed DOT did not return the typed parse error."
        );
    }
}

// Runs all Graphviz parsing and layout checks.
int main()
{
    return test_attribute_parsing() && test_graphviz_layout() && test_graphviz_errors() ? 0 : 1;
}
