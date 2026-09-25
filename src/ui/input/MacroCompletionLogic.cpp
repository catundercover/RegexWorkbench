// Implements token detection, filtering, and replacement for regex macros.
#include "ui/input/MacroCompletionLogic.hpp"

#include <algorithm>
#include <array>
#include <cctype>

namespace ui::input
{
    namespace
    {
        // Supported textual macros in stable display order for finite regexes.
        constexpr std::array FiniteMacroSuggestions{
            MacroSuggestion{"sigma", "Σ"},
            MacroSuggestion{"epsilon", "ε"},
            MacroSuggestion{"emptyset", "∅"}
        };

        // Supported textual macros in stable display order for omega regexes.
        constexpr std::array OmegaMacroSuggestions{
            MacroSuggestion{"sigma", "Σ"},
            MacroSuggestion{"epsilon", "ε"},
            MacroSuggestion{"emptyset", "∅"},
            MacroSuggestion{"omega", "^ω"}
        };

        // Returns whether a byte may occur after a macro's backslash.
        bool is_macro_character(char character)
        {
            return std::isalnum(static_cast<unsigned char>(character)) || character == '_';
        }

        template <typename Suggestions>
        void collect_matches(
            const Suggestions& suggestions,
            std::string_view prefix,
            std::vector<MacroSuggestion>& matches
        )
        {
            for (const MacroSuggestion& suggestion : suggestions)
            {
                if (prefix.empty() || suggestion.trigger.starts_with(prefix))
                {
                    matches.push_back(suggestion);
                }
            }
        }
    }

    std::optional<ActiveMacroToken>
    find_active_macro_token(std::string_view text, std::size_t cursor_position)
    {
        cursor_position = std::min(cursor_position, text.size());
        if (cursor_position == 0)
        {
            return std::nullopt;
        }

        std::size_t position = cursor_position;
        while (position > 0 && is_macro_character(text[position - 1]))
        {
            --position;
        }

        if (position == 0 || text[position - 1] != '\\')
        {
            return std::nullopt;
        }

        ActiveMacroToken token;
        token.slash_position = position - 1;
        token.prefix = std::string(text.substr(position, cursor_position - position));
        return token;
    }

    std::vector<MacroSuggestion>
    matching_macro_suggestions(std::string_view prefix, app::operations::ExpressionFlavor flavor)
    {
        std::vector<MacroSuggestion> matches;

        if (flavor == app::operations::ExpressionFlavor::OmegaRegex)
        {
            collect_matches(OmegaMacroSuggestions, prefix, matches);
        }
        else
        {
            collect_matches(FiniteMacroSuggestions, prefix, matches);
        }

        return matches;
    }

    bool replace_macro_token(
        std::string& text,
        std::size_t slash_position,
        std::size_t cursor_position,
        std::string_view replacement,
        std::size_t& new_cursor_position
    )
    {
        cursor_position = std::min(cursor_position, text.size());
        if (slash_position > cursor_position || slash_position >= text.size() ||
            text[slash_position] != '\\')
        {
            return false;
        }

        text.replace(
            slash_position, cursor_position - slash_position, replacement.data(), replacement.size()
        );
        new_cursor_position = slash_position + replacement.size();
        return true;
    }
}
