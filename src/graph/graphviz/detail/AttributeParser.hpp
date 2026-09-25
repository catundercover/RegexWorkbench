// Declares parsers for geometry stored in Graphviz attributes.
#pragma once

#include "graph/model/Layout.hpp"

#include <optional>
#include <string_view>
#include <vector>

namespace graph::graphviz::detail
{
    // Contains cubic spline segments and Graphviz's optional explicit arrow tip.
    struct ParsedSpline
    {
        std::vector<CubicBezier> segments;
        std::optional<Point> arrow_tip;
    };

    // Parses a comma-separated Graphviz point.
    [[nodiscard]] std::optional<Point> parse_point(std::string_view text);
    // Parses a Graphviz bounding box into minimum and maximum points.
    [[nodiscard]] std::optional<Bounds> parse_bounds(std::string_view text);
    // Parses a Graphviz edge position into cubic Bezier segments.
    [[nodiscard]] std::optional<ParsedSpline> parse_spline(std::string_view text);
    // Maps Graphviz label-justification text to the UI alignment model.
    [[nodiscard]] LabelAlignment parse_label_alignment(std::string_view text);
}
