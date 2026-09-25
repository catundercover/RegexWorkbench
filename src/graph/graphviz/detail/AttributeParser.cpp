// Implements strict parsing of Graphviz geometry attributes.
#include "graph/graphviz/detail/AttributeParser.hpp"

#include <array>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <string_view>
#include <vector>

namespace graph::graphviz::detail
{
    namespace
    {
        // Parses an exact number of finite comma-separated coordinates.
        template <std::size_t Size>
        std::optional<std::array<float, Size>> parse_coordinates(const std::string_view text)
        {
            std::array<float, Size> values{};
            std::size_t begin = 0;

            for (std::size_t index = 0; index < Size; ++index)
            {
                const std::size_t end = index + 1 == Size ? text.size() : text.find(',', begin);
                if (end == std::string_view::npos || end == begin)
                {
                    return std::nullopt;
                }

                const std::string_view coordinate = text.substr(begin, end - begin);
                const auto parsed = std::from_chars(
                    coordinate.data(),
                    coordinate.data() + coordinate.size(),
                    values[index],
                    std::chars_format::general
                );
                if (parsed.ec != std::errc{} ||
                    parsed.ptr != coordinate.data() + coordinate.size() ||
                    !std::isfinite(values[index]))
                {
                    return std::nullopt;
                }
                begin = end + 1;
            }

            if (begin != text.size() + 1)
            {
                return std::nullopt;
            }
            return values;
        }

        // Splits Graphviz attribute text without allocating token strings.
        std::vector<std::string_view> whitespace_separated(const std::string_view text)
        {
            std::vector<std::string_view> tokens;
            std::size_t begin = 0;
            while (begin < text.size())
            {
                while (begin < text.size() && (text[begin] == ' ' || text[begin] == '\t' ||
                                               text[begin] == '\n' || text[begin] == '\r'))
                {
                    ++begin;
                }
                if (begin == text.size())
                {
                    break;
                }

                std::size_t end = begin;
                while (end < text.size() && text[end] != ' ' && text[end] != '\t' &&
                       text[end] != '\n' && text[end] != '\r')
                {
                    ++end;
                }
                tokens.push_back(text.substr(begin, end - begin));
                begin = end;
            }
            return tokens;
        }
    }

    std::optional<Point> parse_point(const std::string_view text)
    {
        const auto coordinates = parse_coordinates<2>(text);
        if (!coordinates)
        {
            return std::nullopt;
        }
        return Point{(*coordinates)[0], (*coordinates)[1]};
    }

    std::optional<Bounds> parse_bounds(const std::string_view text)
    {
        const auto coordinates = parse_coordinates<4>(text);
        if (!coordinates || (*coordinates)[0] > (*coordinates)[2] ||
            (*coordinates)[1] > (*coordinates)[3])
        {
            return std::nullopt;
        }
        return Bounds{
            Point{(*coordinates)[0], (*coordinates)[1]}, Point{(*coordinates)[2], (*coordinates)[3]}
        };
    }

    std::optional<ParsedSpline> parse_spline(const std::string_view text)
    {
        ParsedSpline result;
        std::vector<Point> points;

        for (const std::string_view token : whitespace_separated(text))
        {
            if (token.starts_with("e,") || token.starts_with("s,"))
            {
                const std::optional<Point> endpoint = parse_point(token.substr(2));
                if (!endpoint)
                {
                    return std::nullopt;
                }
                if (token.front() == 'e')
                {
                    result.arrow_tip = endpoint;
                }
                continue;
            }

            const std::optional<Point> point = parse_point(token);
            if (!point)
            {
                return std::nullopt;
            }
            points.push_back(*point);
        }

        if (points.empty())
        {
            return result;
        }
        if (points.size() < 4 || (points.size() - 1) % 3 != 0)
        {
            return std::nullopt;
        }

        result.segments.reserve((points.size() - 1) / 3);
        for (std::size_t index = 0; index + 3 < points.size(); index += 3)
        {
            result.segments.push_back(
                CubicBezier{points[index], points[index + 1], points[index + 2], points[index + 3]}
            );
        }
        return result;
    }

    LabelAlignment parse_label_alignment(const std::string_view text)
    {
        if (text == "l")
        {
            return LabelAlignment::Left;
        }
        if (text == "r")
        {
            return LabelAlignment::Right;
        }
        return LabelAlignment::Center;
    }
}
