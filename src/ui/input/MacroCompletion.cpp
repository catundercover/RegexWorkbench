// Implements keyboard and popup behavior for regex macro completion.
#include "ui/input/MacroCompletion.hpp"

#include "ui/input/MacroCompletionLogic.hpp"
#include "ui/style/ApplicationTheme.hpp"

#include <algorithm>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <string>
#include <utility>
#include <vector>

namespace ui::input
{
    namespace
    {
        // Callback modes needed for live filtering, navigation, and completion.
        constexpr ImGuiInputTextFlags MacroInputFlags =
            ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_CallbackHistory |
            ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_EnterReturnsTrue;

        // Recomputes completion visibility and selection for the current cursor.
        void update_completion(
            MacroCompletionState& state, const std::string& text, std::size_t cursor_position
        )
        {
            const std::optional<ActiveMacroToken> token =
                find_active_macro_token(text, cursor_position);
            if (!token.has_value())
            {
                reset_macro_completion(state);
                return;
            }

            std::vector<MacroSuggestion> matches =
                matching_macro_suggestions(token->prefix, state.flavor);
            if (matches.empty())
            {
                reset_macro_completion(state);
                return;
            }

            const bool was_showing = state.show_suggestions;
            state.show_suggestions = true;
            state.current_prefix = token->prefix;
            state.slash_position = token->slash_position;
            state.cursor_position = cursor_position;

            if (!was_showing || state.selected_index >= matches.size())
            {
                state.selected_index = 0;
            }
        }

        // Returns current matches while repairing stale selection state.
        std::vector<MacroSuggestion> current_matches(MacroCompletionState& state)
        {
            std::vector<MacroSuggestion> matches =
                matching_macro_suggestions(state.current_prefix, state.flavor);
            if (matches.empty())
            {
                reset_macro_completion(state);
                return {};
            }

            if (state.selected_index >= matches.size())
            {
                state.selected_index = 0;
            }

            return matches;
        }

        // Replaces a macro directly through Dear ImGui's active text buffer.
        void insert_from_callback(
            ImGuiInputTextCallbackData* data,
            MacroCompletionState& state,
            std::string_view replacement
        )
        {
            if (state.slash_position == std::string::npos)
            {
                return;
            }

            const int slash_position = static_cast<int>(state.slash_position);
            const int cursor_position = data->CursorPos;
            if (cursor_position < slash_position)
            {
                return;
            }

            const std::string replacement_text(replacement);
            data->DeleteChars(slash_position, cursor_position - slash_position);
            data->InsertChars(slash_position, replacement_text.c_str());

            const int new_cursor_position = slash_position + static_cast<int>(replacement.size());
            data->CursorPos = new_cursor_position;
            data->SelectionStart = new_cursor_position;
            data->SelectionEnd = new_cursor_position;

            state.cursor_position = static_cast<std::size_t>(new_cursor_position);
            state.refocus_input = true;
            state.keyboard_completion_accepted = true;
            reset_macro_completion(state);
        }

        // Handles live token updates, keyboard selection, and insertion.
        int macro_input_callback(ImGuiInputTextCallbackData* data)
        {
            auto* state = static_cast<MacroCompletionState*>(data->UserData);
            if (state == nullptr)
            {
                return 0;
            }

            state->cursor_position = static_cast<std::size_t>(data->CursorPos);
            const std::string current_text(data->Buf, static_cast<std::size_t>(data->BufTextLen));

            if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
            {
                update_completion(*state, current_text, state->cursor_position);

                if (state->show_suggestions && (ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
                                                ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false)))
                {
                    const std::vector<MacroSuggestion> matches = current_matches(*state);
                    if (!matches.empty())
                    {
                        insert_from_callback(
                            data, *state, matches[state->selected_index].replacement
                        );
                        return 1;
                    }
                }

                return 0;
            }

            if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
            {
                if (!state->show_suggestions)
                {
                    return 0;
                }

                const std::vector<MacroSuggestion> matches = current_matches(*state);
                if (matches.empty())
                {
                    return 0;
                }

                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    state->selected_index =
                        state->selected_index == 0 ? matches.size() - 1 : state->selected_index - 1;
                    return 1;
                }

                if (data->EventKey == ImGuiKey_DownArrow)
                {
                    state->selected_index = (state->selected_index + 1) % matches.size();
                    return 1;
                }

                return 0;
            }

            if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion &&
                state->show_suggestions)
            {
                const std::vector<MacroSuggestion> matches = current_matches(*state);
                if (!matches.empty())
                {
                    insert_from_callback(data, *state, matches[state->selected_index].replacement);
                    return 1;
                }
            }

            return 0;
        }

        // Replaces the active macro after a pointer selection in the overlay.
        bool insert_from_popup(
            MacroCompletionState& state, std::string& text, std::string_view replacement
        )
        {
            std::size_t new_cursor_position = state.cursor_position;
            if (!replace_macro_token(
                    text,
                    state.slash_position,
                    state.cursor_position,
                    replacement,
                    new_cursor_position
                ))
            {
                return false;
            }

            state.cursor_position = new_cursor_position;
            state.refocus_input = true;
            reset_macro_completion(state);
            return true;
        }
    }

    void reset_macro_completion(MacroCompletionState& state)
    {
        state.show_suggestions = false;
        state.current_prefix.clear();
        state.selected_index = 0;
        state.slash_position = std::string::npos;
        state.popup_min = ImVec2();
        state.popup_max = ImVec2();
    }

    MacroTextInputResult render_macro_text_input(
        const char* input_id, const char* hint, std::string& text, MacroCompletionState& state
    )
    {
        if (state.refocus_input)
        {
            ImGui::SetKeyboardFocusHere();
            state.refocus_input = false;
        }

        state.keyboard_completion_accepted = false;
        const bool enter_pressed = ImGui::InputTextWithHint(
            input_id, hint, &text, MacroInputFlags, macro_input_callback, &state
        );
        const bool changed = ImGui::IsItemEdited();

        state.input_min = ImGui::GetItemRectMin();
        state.input_max = ImGui::GetItemRectMax();
        return MacroTextInputResult{changed, enter_pressed && !state.keyboard_completion_accepted};
    }

    bool render_macro_suggestions(MacroCompletionState& state, std::string& text)
    {
        if (!state.show_suggestions)
        {
            return false;
        }

        const std::vector<MacroSuggestion> matches = current_matches(state);
        if (matches.empty())
        {
            return false;
        }

        constexpr float PaddingX = 10.0f;
        constexpr float PaddingY = 6.0f;
        constexpr float RowPaddingY = 4.0f;
        constexpr float Rounding = 4.0f;

        const ImVec2 position(state.input_min.x, state.input_max.y + 2.0f);
        float width = 0.0f;
        std::vector<std::string> labels;
        labels.reserve(matches.size());

        for (const MacroSuggestion& suggestion : matches)
        {
            std::string label = "\\";
            label += suggestion.trigger;
            label += "  →  ";
            label += suggestion.replacement;
            width = std::max(width, ImGui::CalcTextSize(label.c_str()).x);
            labels.push_back(std::move(label));
        }

        width += PaddingX * 2.0f;
        const float row_height = ImGui::GetTextLineHeight() + RowPaddingY * 2.0f;
        const float height = PaddingY * 2.0f + row_height * static_cast<float>(matches.size());
        const ImRect popup_bounds(position, ImVec2(position.x + width, position.y + height));
        state.popup_min = popup_bounds.Min;
        state.popup_max = popup_bounds.Max;

        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
        const style::ApplicationPalette& palette = style::application_palette();
        draw_list->AddRectFilled(
            popup_bounds.Min, popup_bounds.Max, ImGui::GetColorU32(palette.card), Rounding
        );
        draw_list->AddRect(
            popup_bounds.Min, popup_bounds.Max, ImGui::GetColorU32(palette.border), Rounding
        );

        const ImVec2 mouse_position = ImGui::GetMousePos();
        const bool mouse_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

        for (std::size_t index = 0; index < matches.size(); ++index)
        {
            const float row_y = position.y + PaddingY + row_height * static_cast<float>(index);
            const ImRect row_bounds(
                ImVec2(position.x, row_y), ImVec2(position.x + width, row_y + row_height)
            );
            const bool hovered = row_bounds.Contains(mouse_position);

            if (hovered)
            {
                state.selected_index = index;
            }

            if (hovered || index == state.selected_index)
            {
                draw_list->AddRectFilled(
                    row_bounds.Min,
                    row_bounds.Max,
                    ImGui::GetColorU32(hovered ? palette.info_surface : palette.subtle_surface),
                    Rounding
                );
            }

            draw_list->AddText(
                ImVec2(position.x + PaddingX, row_y + RowPaddingY),
                ImGui::GetColorU32(ImGuiCol_Text),
                labels[index].c_str()
            );

            if (hovered && mouse_clicked)
            {
                state.selected_index = index;
                return insert_from_popup(state, text, matches[index].replacement);
            }
        }

        if (mouse_clicked)
        {
            const ImRect input_bounds(state.input_min, state.input_max);
            if (!input_bounds.Contains(mouse_position) && !popup_bounds.Contains(mouse_position))
            {
                reset_macro_completion(state);
            }
        }

        return false;
    }
}
