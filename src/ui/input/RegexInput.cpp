// Implements the composed regex editor and generation control.
#include "ui/input/RegexInput.hpp"

#include "ui/input/RandomRegexControls.hpp"
#include "ui/style/ApplicationTheme.hpp"

#include <imgui.h>
#include <utility>

namespace ui::input
{
    void refresh_syntax_feedback(RegexInputState& state)
    {
        SyntaxFeedback feedback = validate_expression_syntax(state.text, state.macro.flavor);
        state.syntax_validity = feedback.validity;
        state.syntax_message = std::move(feedback.message);
    }

    RegexInputResult render_regex_input(
        RegexInputState& state,
        RandomRegexState& random_state,
        const app::operations::ExpressionFlavor flavor,
        const regex::generation::Config& finite_config,
        const regex::generation::OmegaConfig& omega_config,
        const RegexInputConfig& config
    )
    {
        int pushed_colors = 0;
        if (config.background_color.has_value())
        {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, *config.background_color);
            ++pushed_colors;
        }

        const bool invalid = state.syntax_validity == SyntaxValidity::Invalid;
        if (invalid)
        {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, style::application_palette().error_surface);
            ImGui::PushStyleColor(ImGuiCol_Border, style::application_palette().error);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0F);
            pushed_colors += 2;
        }

        const MacroTextInputResult text_result =
            render_macro_text_input(config.input_id, config.hint, state.text, state.macro);

        if (invalid)
        {
            ImGui::PopStyleVar();
        }
        ImGui::PopStyleColor(pushed_colors);

        const bool generated = render_random_regex_button(
            config.random_button_id,
            state.text,
            state.macro,
            random_state,
            flavor,
            finite_config,
            omega_config
        );
        if (text_result.changed || generated)
        {
            refresh_syntax_feedback(state);
        }
        return RegexInputResult{
            text_result.changed || generated, text_result.submit_requested, generated
        };
    }

    bool render_regex_input_suggestions(RegexInputState& state)
    {
        return render_macro_suggestions(state.macro, state.text);
    }
}