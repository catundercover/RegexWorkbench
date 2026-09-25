// Declares random-regex generation controls shared by the UI.
#pragma once

#include "regex/generation/RandomOmegaRegexGenerator.hpp"
#include "regex/generation/RandomRegexGenerator.hpp"
#include "app/operations/ExpressionFlavor.hpp"
#include <string>

namespace ui::input
{
    struct MacroCompletionState;

    // Owns the generator state shared by all random-regex buttons.
    struct RandomRegexState
    {
        regex::generation::Generator generator;
        regex::generation::OmegaGenerator omega_generator;
    };

    // Renders a generation button; returns true when it replaces the regex text.
    [[nodiscard]] bool render_random_regex_button(
        const char* button_id,
        std::string& text,
        MacroCompletionState& macro_state,
        RandomRegexState& random_state,
        app::operations::ExpressionFlavor flavor,
        const regex::generation::Config& finite_config,
        const regex::generation::OmegaConfig& omega_config
    );

    // Renders the reusable finite-generator controls and reports whether they changed.
    [[nodiscard]] bool render_random_regex_config(regex::generation::Config& config);

    // Renders the reusable omega-generator controls and reports whether they changed.
    [[nodiscard]] bool render_random_omega_regex_config(regex::generation::OmegaConfig& config);

    // Renders the shared generator-settings popup when it is open.
    void render_random_regex_settings_popup(
        regex::generation::Config& finite_config,
        regex::generation::OmegaConfig& omega_config
    );
}