// Implements line and column derivation for parser diagnostics.
#include "regex/parsing/ParsingDiagnostics.hpp"

#include <algorithm>
#include <sstream>

namespace regex::parsing::detail
{
    namespace
    {
        // Counts UTF-8 code points by excluding continuation bytes.
        std::size_t code_point_count(const std::string_view text)
        {
            return static_cast<std::size_t>(std::count_if(
                text.begin(),
                text.end(),
                [](const char byte) { return (static_cast<unsigned char>(byte) & 0xC0U) != 0x80U; }
            ));
        }
    }

    ParseError make_parse_error(
        const std::string_view input,
        const std::size_t requested_offset,
        const std::string& description
    )
    {
        ParseError result;
        result.byte_offset = std::min(requested_offset, input.size());

        const std::size_t line_start_search = result.byte_offset == 0 ? 0 : result.byte_offset - 1;
        const std::size_t newline_before = input.rfind('\n', line_start_search);
        const std::size_t line_start =
            newline_before == std::string_view::npos ? 0 : newline_before + 1;
        const std::size_t newline_after = input.find('\n', result.byte_offset);
        const std::size_t line_end =
            newline_after == std::string_view::npos ? input.size() : newline_after;

        const std::string_view preceding_lines = input.substr(0, line_start);
        result.line = 1 + static_cast<std::size_t>(
                              std::count(preceding_lines.begin(), preceding_lines.end(), '\n')
                          );
        result.column =
            1 + code_point_count(input.substr(line_start, result.byte_offset - line_start));

        std::ostringstream message;
        message << description << "\n\n"
                << input.substr(line_start, line_end - line_start) << '\n'
                << std::string(result.column - 1, ' ') << '^';
        result.message = message.str();
        return result;
    }
}
