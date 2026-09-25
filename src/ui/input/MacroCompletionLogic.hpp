// Defines rendering-independent regex macro completion logic.
#pragma once

#include "app/operations/ExpressionFlavor.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ui::input
{
    // Maps a typed macro trigger to its Unicode replacement.
    struct MacroSuggestion
    {
        std::string_view trigger;
        std::string_view replacement;
    };

    // Identifies the incomplete macro immediately before the cursor.
    struct ActiveMacroToken
    {
        std::size_t slash_position = 0;
        std::string prefix;
    };

    // Finds the backslash token directly before the cursor, if one is active.
    [[nodiscard]] std::optional<ActiveMacroToken>
    find_active_macro_token(std::string_view text, std::size_t cursor_position);

    // Returns macro entries whose trigger starts with the supplied prefix.
    [[nodiscard]] std::vector<MacroSuggestion>
    matching_macro_suggestions(std::string_view prefix, app::operations::ExpressionFlavor flavor);

    // Replaces a validated macro range and reports the new UTF-8 byte cursor position.
    [[nodiscard]] bool replace_macro_token(
        std::string& text,
        std::size_t slash_position,
        std::size_t cursor_position,
        std::string_view replacement,
        std::size_t& new_cursor_position
    );
}
