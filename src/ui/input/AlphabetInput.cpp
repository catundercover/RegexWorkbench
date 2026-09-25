// Implements validation and rendering for explicit alphabet input.
#include "ui/input/AlphabetInput.hpp"

#include "app/operations/OperationValidation.hpp"
#include "ui/style/ApplicationTheme.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

namespace ui::input
{
    AlphabetInputResult render_alphabet_input(AlphabetInputState& state)
    {
        const bool toggled = ImGui::Checkbox("Σ  Additional alphabet", &state.enabled);
        bool changed = toggled;
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Include symbols that do not occur explicitly in the expression.");
        }
        if (!state.enabled)
        {
            state.validation_error.reset();
            return AlphabetInputResult{changed, false};
        }
        if (toggled)
        {
            state.validation_error = app::operations::parse_extra_alphabet(state.text).error;
        }

        ImGui::SetNextItemWidth(-1.0F);
        const bool enter_pressed = ImGui::InputTextWithHint(
            "##extra_alphabet_input",
            "Additional symbols, e.g. a b c",
            &state.text,
            ImGuiInputTextFlags_EnterReturnsTrue
        );
        const bool text_changed = ImGui::IsItemEdited();
        changed = changed || text_changed;
        if (text_changed)
        {
            state.validation_error = app::operations::parse_extra_alphabet(state.text).error;
        }

        if (state.validation_error.has_value())
        {
            ImGui::TextColored(
                style::application_palette().error, "%s", state.validation_error->c_str()
            );
        }
        else
        {
            ImGui::TextDisabled(
                "Symbols used by the expression are included automatically; spaces are ignored."
            );
        }

        return AlphabetInputResult{changed, enter_pressed};
    }
}