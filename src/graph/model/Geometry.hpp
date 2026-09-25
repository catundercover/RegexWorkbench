// Defines rendering-independent graph geometry primitives.
#pragma once

namespace graph
{
    // Two-dimensional point in Graphviz layout coordinates.
    struct Point
    {
        float x = 0.0F;
        float y = 0.0F;

        // Compares both coordinates exactly.
        bool operator==(const Point&) const = default;
    };

    // Axis-aligned minimum and maximum graph extents.
    struct Bounds
    {
        Point minimum;
        Point maximum{1.0F, 1.0F};

        // Compares both boundary points exactly.
        bool operator==(const Bounds&) const = default;
    };

    // Cubic Bezier curve represented by endpoints and two control points.
    struct CubicBezier
    {
        Point start;
        Point first_control;
        Point second_control;
        Point end;

        // Compares all four curve points exactly.
        bool operator==(const CubicBezier&) const = default;
    };
}
