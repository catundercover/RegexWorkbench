// Defines source-aware diagnostics for regular-expression parsing.
#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace regex::parsing
{
    // Describes a parser error in both byte and human-readable coordinates.
    struct ParseError
    {
        std::size_t byte_offset = 0;
        std::size_t line = 1;
        std::size_t column = 1;
        std::string message;
    };

    namespace detail
    {
        // Clamps an offset and derives its one-based line and column.
        [[nodiscard]] ParseError make_parse_error(
            std::string_view input, std::size_t requested_offset, const std::string& description
        );
    }
}
