// Defines reusable regular-expression input state and rendering.
#pragma once

#include "ui/input/ExpressionInputValidation.hpp"
#include "ui/input/MacroCompletion.hpp"

#include <imgui.h>
#include <optional>
#include <string>

namespace regex::generation
{
    struct Config;
    struct OmegaConfig;
}

namespace ui::input
{
    struct RandomRegexState;

    // Stores editable regex text and its macro-completion state.
    struct RegexInputState
    {
        std::string text;
        MacroCompletionState macro;
        SyntaxValidity syntax_validity = SyntaxValidity::Empty;
        std::string syntax_message;
    };

    // Supplies stable widget identifiers and optional field styling.
    struct RegexInputConfig
    {
        const char* input_id;
        const char* random_button_id;
        const char* hint;
        std::optional<ImVec4> background_color;
    };

    // Reports regex edits, random generation, and explicit submit requests caused by Enter.
    struct RegexInputResult
    {
        bool changed = false;
        bool run_requested = false;
        bool generated = false;
    };

    // Renders one regex field and its random-generation control.
    [[nodiscard]] RegexInputResult render_regex_input(
        RegexInputState& state,
        RandomRegexState& random_state,
        app::operations::ExpressionFlavor flavor,
        const regex::generation::Config& finite_config,
        const regex::generation::OmegaConfig& omega_config,
        const RegexInputConfig& config
    );

    // Refreshes source-aware syntax feedback after text or language-domain changes.
    void refresh_syntax_feedback(RegexInputState& state);

    // Renders suggestions above the main window; returns true after inserting one.
    [[nodiscard]] bool render_regex_input_suggestions(RegexInputState& state);
}
