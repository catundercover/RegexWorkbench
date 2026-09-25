// Declares regex input normalization and source-offset mapping.
#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace regex::parsing
{
    // Removes whitespace and accepts boolean and superscript input aliases.
    [[nodiscard]] std::string normalize_input(std::string_view input);

    namespace detail
    {
        // Stores normalized text and maps its bytes back to the original input.
        struct NormalizedInput
        {
            std::string text;
            std::vector<std::size_t> source_offsets;
            std::size_t source_size = 0;

            // Maps a normalized byte offset to the corresponding original offset.
            [[nodiscard]] std::size_t original_offset(std::size_t normalized_offset) const;
        };

        // Normalizes input while preserving an offset for each retained byte.
        [[nodiscard]] NormalizedInput normalize_with_mapping(std::string_view input);
    }
}
