// Defines Dear ImGui integration for regex macro completion.
#pragma once

#include "app/operations/ExpressionFlavor.hpp"

#include <cstddef>
#include <imgui.h>
#include <string>

namespace ui::input
{
    // Stores the transient selection, cursor, and popup geometry of a completion.
    struct MacroCompletionState
    {
        app::operations::ExpressionFlavor flavor = app::operations::ExpressionFlavor::FiniteRegex;
        bool show_suggestions = false;
        std::string current_prefix;
        std::size_t selected_index = 0;
        std::size_t slash_position = std::string::npos;
        std::size_t cursor_position = 0;
        ImVec2 input_min;
        ImVec2 input_max;
        ImVec2 popup_min;
        ImVec2 popup_max;
        bool refocus_input = false;
        bool keyboard_completion_accepted = false;
    };

    // Reports edits and an explicit Enter request from one text input frame.
    struct MacroTextInputResult
    {
        bool changed = false;
        bool submit_requested = false;
    };

    // Closes completion and resets its transient selection state.
    void reset_macro_completion(MacroCompletionState& state);

    // Renders an input with keyboard-driven macro completion.
    [[nodiscard]] MacroTextInputResult render_macro_text_input(
        const char* input_id, const char* hint, std::string& text, MacroCompletionState& state
    );

    // Renders the suggestion overlay; returns true after inserting a suggestion.
    [[nodiscard]] bool render_macro_suggestions(MacroCompletionState& state, std::string& text);
}
