// Defines state and rendering for the optional alphabet input.
#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace ui::input
{
    // Stores the optional alphabet editor state and its latest validation error.
    struct AlphabetInputState
    {
        std::string text;
        std::optional<std::string> validation_error;
        bool enabled = false;
    };
    // Reports alphabet edits and explicit submit requests caused by Enter.
    struct AlphabetInputResult
    {
        bool changed = false;
        bool run_requested = false;
    };

    // Renders a sigma toggle and, when enabled, the additional-alphabet field.
    [[nodiscard]] AlphabetInputResult render_alphabet_input(AlphabetInputState& state);

    // Returns the additional symbols only while the alphabet field is enabled.
    [[nodiscard]] inline std::string_view active_alphabet(const AlphabetInputState& state)
    {
        return state.enabled ? std::string_view(state.text) : std::string_view{};
    }
}
