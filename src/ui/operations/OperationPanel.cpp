/// @file
/// Implements operation selection, progressive options, and explicit start controls.
#include "ui/operations/OperationPanel.hpp"

#include "ui/components/UiComponents.hpp"
#include "ui/input/AlphabetInput.hpp"
#include "ui/input/RegexInput.hpp"
#include "ui/operations/OperationCoordinator.hpp"
#include "ui/operations/OperationState.hpp"
#include "ui/style/ApplicationTheme.hpp"

#include <algorithm>
#include <array>
#include <imgui.h>
#include <optional>

namespace ui::operations
{
    namespace
    {
        /// Associates one navigation label with its operation mode.
        struct ModeOption
        {
            const char* label;
            Mode mode;
        };

        /// Operation modes in their stable UI order.
        constexpr std::array<ModeOption, 3> ModeOptions = {
            ModeOption{"TRANSLATE", Mode::Translate},
            ModeOption{"REWRITE", Mode::Rewrite},
            ModeOption{"COMPARE", Mode::Compare}
        };

        /// Returns a low-emphasis input tint for the first compared language.
        ImVec4 compare_left_color()
        {
            ImVec4 color = style::application_palette().primary;
            color.w = 0.10F;
            return color;
        }

        /// Returns a low-emphasis input tint for the second compared language.
        ImVec4 compare_right_color()
        {
            ImVec4 color = style::application_palette().accent;
            color.w = 0.10F;
            return color;
        }

        /// Renders the operation navigation as equal-width underline tabs.
        void render_mode_selector(OperationState& state, OperationCoordinator& coordinator)
        {
            const float spacing = ImGui::GetStyle().ItemSpacing.x;
            const float tab_width = (ImGui::GetContentRegionAvail().x - spacing * 2.0F) /
                                    static_cast<float>(ModeOptions.size());

            for (std::size_t index = 0; index < ModeOptions.size(); ++index)
            {
                if (index > 0)
                {
                    ImGui::SameLine();
                }

                const ModeOption& option = ModeOptions[index];
                if (components::tab_button(
                        option.label,
                        option.mode == state.mode,
                        ImVec2(tab_width, ImGui::GetFrameHeight() + 5.0F)
                    ) &&
                    option.mode != state.mode)
                {
                    state.mode = option.mode;
                    coordinator.clear(state);
                }
            }
        }

        /// Renders one expression editor and invalidates output after an edit.
        [[nodiscard]] input::RegexInputResult render_expression_input(
            OperationState& state,
            input::RegexInputState& expression,
            const OperationPanelInputs& inputs,
            OperationCoordinator& coordinator,
            const char* label,
            const char* hint,
            const char* input_id,
            const char* random_button_id,
            const std::optional<ImVec4> background_color = std::nullopt
        )
        {
            ImGui::TextUnformatted(label);
            const float random_button_width = ImGui::GetFrameHeight();
            ImGui::SetNextItemWidth(
                ImGui::GetContentRegionAvail().x - random_button_width -
                ImGui::GetStyle().ItemSpacing.x
            );

            const input::RegexInputConfig config{
                input_id, random_button_id, hint, background_color
            };
            const input::RegexInputResult result = input::render_regex_input(
                expression,
                inputs.random_regex,
                state.flavor,
                inputs.random_regex_config,
                inputs.random_omega_regex_config,
                config
            );
            if (result.changed)
            {
                coordinator.clear(state);
            }
            return result;
        }

        /// Renders compact source-aware feedback without obscuring macro suggestions.
        void render_syntax_feedback(const input::RegexInputState& expression)
        {
            if (expression.macro.show_suggestions ||
                expression.syntax_validity != input::SyntaxValidity::Invalid)
            {
                return;
            }

            ImGui::TextColored(
                style::application_palette().error, "%s", expression.syntax_message.c_str()
            );
        }

        /// Renders the visible alphabet field shared by all operations.
        [[nodiscard]] bool render_alphabet(
            OperationState& state,
            const OperationPanelInputs& inputs,
            OperationCoordinator& coordinator
        )
        {
            ImGui::Dummy(ImVec2(0.0F, 4.0F));
            const input::AlphabetInputResult result = input::render_alphabet_input(inputs.alphabet);
            if (result.changed)
            {
                coordinator.clear(state);
            }
            return result.run_requested;
        }

        /// Renders finite or omega translation output choices.
        /// Renders finite or omega translation output choices.
        [[nodiscard]] bool
        render_translate_options(OperationState& state, OperationCoordinator& coordinator)
        {
            ImGui::Dummy(ImVec2(0.0F, 6.0F));
            ImGui::TextUnformatted("Automaton");

            const bool omega = state.flavor == app::operations::ExpressionFlavor::OmegaRegex;

            bool changed = false;
            if (ImGui::RadioButton(
                    omega ? "NBA" : "NFA", state.automaton_mode == AutomatonDisplayMode::Nfa
                ) &&
                state.automaton_mode != AutomatonDisplayMode::Nfa)
            {
                state.automaton_mode = AutomatonDisplayMode::Nfa;
                changed = true;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton(
                    omega ? "Minimal DBA" : "Minimal DFA",
                    state.automaton_mode == AutomatonDisplayMode::Dfa
                ) &&
                state.automaton_mode != AutomatonDisplayMode::Dfa)
            {
                state.automaton_mode = AutomatonDisplayMode::Dfa;
                changed = true;
            }

            if (changed)
            {
                coordinator.clear(state);
            }
            return changed;
        }

        /// Renders the operator expansions available to rewrite operations.
        [[nodiscard]] bool
        render_rewrite_options(OperationState& state, OperationCoordinator& coordinator)
        {
            ImGui::Dummy(ImVec2(0.0F, 6.0F));
            components::section_heading(
                "Unabbreviate operators",
                "Selected operators are removed while preserving the represented language."
            );

            bool changed = false;
            const int columns = ImGui::GetContentRegionAvail().x >= 600.0F ? 2 : 1;
            if (ImGui::BeginTable("##rewrite_options", columns, ImGuiTableFlags_SizingStretchSame))
            {
                ImGui::TableNextColumn();
                changed |=
                    ImGui::Checkbox("Complement (!, ~)", &state.rewrite_options.remove_complement);
                ImGui::TableNextColumn();
                changed |= ImGui::Checkbox(
                    "Intersection (&, &&)", &state.rewrite_options.remove_intersection
                );
                ImGui::TableNextColumn();
                changed |= ImGui::Checkbox("Power (^n)", &state.rewrite_options.remove_power);
                ImGui::TableNextColumn();
                changed |= ImGui::Checkbox("One or more (+)", &state.rewrite_options.remove_plus);
                ImGui::TableNextColumn();
                changed |=
                    ImGui::Checkbox("Any symbol (Σ)", &state.rewrite_options.remove_any_symbol);
                ImGui::EndTable();
            }

            if (changed)
            {
                coordinator.clear(state);
            }
            return changed;
        }

        /// Returns the action label associated with the selected operation.
        const char* action_label(const Mode mode)
        {
            switch (mode)
            {
            case Mode::Translate:
                return "Translate";
            case Mode::Rewrite:
                return "Rewrite";
            case Mode::Compare:
                return "Compare";
            }
            return "Run";
        }

        /// Starts the active operation with the current inputs.
        void start_operation(
            OperationState& state,
            const OperationPanelInputs& inputs,
            OperationCoordinator& coordinator
        )
        {
            switch (state.mode)
            {
            case Mode::Translate:
                coordinator.start_translate(
                    state, inputs.primary.text, input::active_alphabet(inputs.alphabet)
                );
                break;
            case Mode::Rewrite:
                coordinator.start_rewrite(
                    state, inputs.primary.text, input::active_alphabet(inputs.alphabet)
                );
                break;
            case Mode::Compare:
                coordinator.start_compare(
                    state,
                    inputs.primary.text,
                    inputs.secondary.text,
                    input::active_alphabet(inputs.alphabet)
                );
                break;
            }
        }

        /// Renders the explicit primary action while preserving Enter as a shortcut.
        [[nodiscard]] bool
        render_primary_action(const OperationState& state, const OperationCoordinator& coordinator)
        {
            constexpr float ButtonWidth = 150.0F;
            ImGui::Dummy(ImVec2(0.0F, 8.0F));
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() +
                std::max(0.0F, ImGui::GetContentRegionAvail().x - ButtonWidth)
            );

            ImGui::BeginDisabled(coordinator.running());
            const bool clicked = components::primary_button(
                action_label(state.mode), ImVec2(ButtonWidth, ImGui::GetFrameHeight() + 2.0F)
            );
            ImGui::EndDisabled();
            return clicked;
        }
    }

    void render_operation_panel(
        OperationState& state, const OperationPanelInputs& inputs, OperationCoordinator& coordinator
    )
    {
        render_mode_selector(state, coordinator);
        ImGui::Dummy(ImVec2(0.0F, 12.0F));

        const bool compare = state.mode == Mode::Compare;
        const bool omega = state.flavor == app::operations::ExpressionFlavor::OmegaRegex;
        const input::RegexInputResult primary = render_expression_input(
            state,
            inputs.primary,
            inputs,
            coordinator,
            compare ? "Expression A" : (omega ? "Omega regular expression" : "Regular expression"),
            compare ? "Enter the first expression"
                    : (omega ? "Enter an omega regular expression" : "Enter a regular expression"),
            "##regex1",
            "random_regex_1",
            compare ? std::optional<ImVec4>{compare_left_color()} : std::nullopt
        );
        render_syntax_feedback(inputs.primary);

        // A generated single expression is immediately useful; comparison still waits for both
        // independently generated or edited operands and an explicit Compare action.
        bool submit_requested = primary.run_requested || primary.generated;
        if (compare)
        {
            ImGui::Dummy(ImVec2(0.0F, 4.0F));
            const input::RegexInputResult secondary = render_expression_input(
                state,
                inputs.secondary,
                inputs,
                coordinator,
                "Expression B",
                "Enter the second expression",
                "##regex2",
                "random_regex_2",
                compare_right_color()
            );
            render_syntax_feedback(inputs.secondary);
            submit_requested = submit_requested || secondary.run_requested || secondary.generated;
        }

        submit_requested = submit_requested || render_alphabet(state, inputs, coordinator);

        switch (state.mode)
        {
        case Mode::Translate:
            submit_requested = submit_requested || render_translate_options(state, coordinator);
            break;
        case Mode::Rewrite:
            submit_requested = submit_requested || render_rewrite_options(state, coordinator);
            break;
        case Mode::Compare:
            break;
        }

        const bool button_clicked = render_primary_action(state, coordinator);
        if ((submit_requested || button_clicked) && !coordinator.running())
        {
            start_operation(state, inputs, coordinator);
        }
    }
}
