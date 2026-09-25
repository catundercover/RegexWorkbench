/// @file
/// Implements composition and event routing for the main UI window.
#include "ui/MainWindow.hpp"

#include "app/operations/ExpressionFlavor.hpp"
#include "app/operations/OperationController.hpp"
#include "ui/components/UiComponents.hpp"
#include "ui/input/RandomRegexControls.hpp"
#include "ui/input/RegexInput.hpp"
#include "ui/operations/OperationPanel.hpp"
#include "ui/operations/OperationState.hpp"
#include "ui/results/OperationOutputView.hpp"
#include "ui/settings/SettingsPage.hpp"
#include "ui/style/ApplicationTheme.hpp"

#include <algorithm>
#include <imgui.h>
#include <variant>

namespace ui
{
    namespace
    {
        /// Applies one word-domain selection and clears output derived from the previous domain.
        void select_flavor(
            MainWindowState& window,
            operations::OperationCoordinator& coordinator,
            const app::operations::ExpressionFlavor flavor
        )
        {
            if (window.operation.flavor == flavor)
            {
                return;
            }

            window.operation.flavor = flavor;
            window.primary_input.macro.flavor = flavor;
            window.secondary_input.macro.flavor = flavor;
            input::reset_macro_completion(window.primary_input.macro);
            input::reset_macro_completion(window.secondary_input.macro);
            input::refresh_syntax_feedback(window.primary_input);
            input::refresh_syntax_feedback(window.secondary_input);
            coordinator.clear(window.operation);
        }

        /// Renders the compact product header and reports a settings focus request.
        [[nodiscard]] bool render_app_header(MainWindowState& window)
        {
            bool settings_focus_requested = false;
            if (!ImGui::BeginTable("##application_header", 2, ImGuiTableFlags_SizingStretchProp))
            {
                return false;
            }

            ImGui::TableSetupColumn("##title", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("##actions", ImGuiTableColumnFlags_WidthFixed, 180.0F);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetWindowFontScale(1.35F);
            ImGui::TextUnformatted("Regex Workbench");
            ImGui::SetWindowFontScale(1.0F);
            ImGui::TextDisabled("A tool for automatic analysis and comparison of regular expressions");

            ImGui::TableNextColumn();
            const float actions_width =
                ImGui::CalcTextSize("Help").x + ImGui::CalcTextSize("Settings").x +
                ImGui::GetStyle().FramePadding.x * 4.0F + ImGui::GetStyle().ItemSpacing.x;
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - actions_width > 0.0F
                                              ? ImGui::GetContentRegionAvail().x - actions_width
                                              : 0.0F)
            );

            if (ImGui::Button("Help"))
            {
                window.help_popup_requested = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Settings"))
            {
                window.settings_visible = true;
                settings_focus_requested = true;
            }
            ImGui::EndTable();
            return settings_focus_requested;
        }

        /// Renders the finite/infinite word-domain segmented selector.
        void render_flavor_selector(
            MainWindowState& window, operations::OperationCoordinator& coordinator
        )
        {
            const float button_width = std::min(
                170.0F, (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5F
            );
            const float group_width = button_width * 2.0F + ImGui::GetStyle().ItemSpacing.x;

            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() +
                (ImGui::GetContentRegionAvail().x - group_width > 0.0F
                     ? (ImGui::GetContentRegionAvail().x - group_width) * 0.5F
                     : 0.0F)
            );

            const bool finite =
                window.operation.flavor == app::operations::ExpressionFlavor::FiniteRegex;
            if (components::segmented_button(
                    "Finite words", finite, ImVec2(button_width, ImGui::GetFrameHeight() + 2.0F)
                ))
            {
                select_flavor(window, coordinator, app::operations::ExpressionFlavor::FiniteRegex);
            }
            ImGui::SameLine();
            if (components::segmented_button(
                    "Infinite words", !finite, ImVec2(button_width, ImGui::GetFrameHeight() + 2.0F)
                ))
            {
                select_flavor(window, coordinator, app::operations::ExpressionFlavor::OmegaRegex);
            }

            ImGui::Dummy(ImVec2(0.0F, 8.0F));
        }

        /// Renders one operator and its meaning in the help syntax table.
        void render_syntax_row(const char* syntax, const char* meaning)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(syntax);
            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", meaning);
        }

        /// Renders the in-product workflow, syntax, alphabet, and graph guide.
        void render_help_popup()
        {
            ImGui::SetNextWindowSize(ImVec2(680.0F, 620.0F), ImGuiCond_Appearing);
            if (!ImGui::BeginPopupModal("Regex Workbench guide", nullptr))
            {
                return;
            }

            const float footer_height = ImGui::GetFrameHeightWithSpacing() + 8.0F;
            ImGui::BeginChild("##guide_content", ImVec2(0.0F, -footer_height));

            ImGui::TextUnformatted("Getting started");
            ImGui::Separator();
            ImGui::BulletText("Choose Finite words or Infinite words.");
            ImGui::BulletText("Select Translate, Rewrite, or Compare and enter the expressions.");
            ImGui::BulletText("Run the operation with its action button or the Enter key.");

            ImGui::Dummy(ImVec2(0.0F, 10.0F));
            ImGui::TextUnformatted("Expression syntax");
            ImGui::Separator();
            if (ImGui::BeginTable(
                    "##syntax_reference",
                    2,
                    ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                        ImGuiTableFlags_SizingStretchProp
                ))
            {
                ImGui::TableSetupColumn("Operator", ImGuiTableColumnFlags_WidthFixed, 120.0F);
                ImGui::TableSetupColumn("Meaning", ImGuiTableColumnFlags_WidthStretch);
                render_syntax_row("rs", "Concatenation (no explicit operator)");
                render_syntax_row("r | s", "Alternation");
                render_syntax_row("r & s", "Intersection");
                render_syntax_row("!r  or  ~r", "Complement");
                render_syntax_row("r*", "Zero or more repetitions");
                render_syntax_row("r+", "One or more repetitions");
                render_syntax_row("r^n", "Exactly n repetitions");
                render_syntax_row("(r)", "Grouping");
                render_syntax_row("ε  ∅  Σ", "Empty word, empty language, and any symbol");
                render_syntax_row("r^ω", "Infinite repetition (Infinite words only)");
                ImGui::EndTable();
            }
            ImGui::Dummy(ImVec2(0.0F, 6.0F));
            ImGui::TextWrapped(
                "Type \\sigma, \\epsilon, or \\emptyset to insert the corresponding "
                "symbol. In Infinite words mode, type \\omega to insert ^ω. "
                "Every alternation and intersection branch must describe infinite words."
            );

            ImGui::Dummy(ImVec2(0.0F, 10.0F));
            ImGui::TextUnformatted("Operations and alphabet");
            ImGui::Separator();
            ImGui::BulletText("Translate constructs an NFA, minimal DFA, NBA, or minimal DBA.");
            ImGui::BulletText(
                "Infinite words: DBA mode reports when no deterministic Buchi automaton exists."
            );
            ImGui::BulletText("Rewrite expands the selected abbreviated operators.");
            ImGui::BulletText("Compare classifies both languages and shows witness words.");
            ImGui::BulletText(
                "The dice generates a random expression for the active word domain."
            );
            ImGui::BulletText("Right-click the dice to configure random generation.");
            ImGui::BulletText(
                "Enable Σ Additional alphabet for symbols absent from the expressions."
            );
            ImGui::TextWrapped(
                "Expression symbols are included automatically. Complement and Σ are interpreted "
                "relative to this complete active alphabet."
            );

            ImGui::Dummy(ImVec2(0.0F, 10.0F));
            ImGui::TextUnformatted("Graph controls");
            ImGui::Separator();
            ImGui::BulletText("Drag a state to place it exactly where you want it.");
            ImGui::BulletText("Drag the background to pan and scroll to zoom.");
            ImGui::BulletText("Fit view preserves moved states, Reset layout restores Graphviz.");
            ImGui::BulletText("Use Graph/Text to inspect or copy the formal definition.");

            ImGui::EndChild();
            ImGui::Separator();
            const float button_width = 100.0F;
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - button_width
            );
            if (components::primary_button("Got it", ImVec2(button_width, 0.0F)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    MainWindow::MainWindow(app::operations::OperationController& operation_controller)
        : operation_controller_(operation_controller), operation_coordinator_(operation_controller)
    {
    }

    void MainWindow::render()
    {
        MainWindowState& window = state_;
        if (applied_color_theme_ != window.settings.color_theme)
        {
            style::apply_application_theme(window.settings.color_theme);
            applied_color_theme_ = window.settings.color_theme;
        }

        window.primary_input.macro.flavor = window.operation.flavor;
        window.secondary_input.macro.flavor = window.operation.flavor;

        ImGuiIO& io = ImGui::GetIO();
        const ImGuiConfigFlags previous_config_flags = io.ConfigFlags;
        const bool macro_suggestions_visible = window.primary_input.macro.show_suggestions ||
                                               window.secondary_input.macro.show_suggestions;
        if (macro_suggestions_visible)
        {
            io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
        }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGui::Begin(
            "Regex Workbench",
            nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings
        );

        operation_coordinator_.poll(window.operation);
        const bool settings_focus_requested = render_app_header(window);
        if (window.help_popup_requested)
        {
            ImGui::OpenPopup("Regex Workbench guide");
            window.help_popup_requested = false;
        }
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0F, 12.0F));

        if (components::begin_centered_card("##operation_card", components::ControlCardWidth))
        {
            render_flavor_selector(window, operation_coordinator_);

            operations::OperationPanelInputs operation_inputs{
                window.primary_input,
                window.secondary_input,
                window.random_regex,
                window.settings.random_regex,
                window.settings.random_omega_regex,
                window.alphabet
            };
            operations::render_operation_panel(
                window.operation, operation_inputs, operation_coordinator_
            );
            input::render_random_regex_settings_popup(
                window.settings.random_regex, window.settings.random_omega_regex
            );
        }
        components::end_centered_card();

        ImGui::Dummy(ImVec2(0.0F, 18.0F));
        results::ResultAction result_action;
        const bool wide_result =
            std::holds_alternative<operations::GraphOutput>(window.operation.output) ||
            std::holds_alternative<operations::ComparisonOutput>(window.operation.output);
        if (components::begin_centered_card(
                "##result_card",
                wide_result ? components::ResultCardWidth : components::ControlCardWidth
            ))
        {
            result_action = results::render_operation_output(
                window.operation.output,
                operation_coordinator_.running(),
                window.settings.graph_canvas_height
            );
        }
        components::end_centered_card();
        ImGui::Dummy(ImVec2(0.0F, 20.0F));

        switch (result_action.kind)
        {
        case results::ResultActionKind::CancelOperation:
            operation_coordinator_.cancel(window.operation);
            break;
        case results::ResultActionKind::None:
            break;
        }

        render_help_popup();
        ImGui::End();

        if (settings::render_settings_page(
                window.settings, window.settings_visible, settings_focus_requested
            ))
        {
            operation_controller_.set_timeout(app::settings::operation_timeout(window.settings));
        }

        const bool inserted_from_primary_popup =
            input::render_regex_input_suggestions(window.primary_input);
        const bool inserted_from_secondary_popup =
            input::render_regex_input_suggestions(window.secondary_input);
        if (inserted_from_primary_popup)
        {
            input::refresh_syntax_feedback(window.primary_input);
        }
        if (inserted_from_secondary_popup)
        {
            input::refresh_syntax_feedback(window.secondary_input);
        }
        if (inserted_from_primary_popup || inserted_from_secondary_popup)
        {
            operation_coordinator_.clear(window.operation);
        }

        io.ConfigFlags = previous_config_flags;
    }
}
