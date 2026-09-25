// Implements random-regex controls and generator settings widgets.
#include "ui/input/RandomRegexControls.hpp"

#include "ui/input/MacroCompletion.hpp"
#include "app/operations/ExpressionFlavor.hpp"

#include <algorithm>
#include <imgui.h>

namespace ui::input
{
    namespace
    {
        // Stable Dear ImGui identifier shared by generator settings buttons.
        constexpr const char* SettingsPopupId = "##random_regex_settings_popup";

        // Converts one weight to its probability within a choice group.
        float probability_from_weights(int weight, int total_weight)
        {
            if (total_weight <= 0)
            {
                return 0.0f;
            }

            return static_cast<float>(weight) / static_cast<float>(total_weight);
        }

        // Renders one generator weight with the common supported range.
        bool render_weight_slider(const char* label, int& weight)
        {
            return ImGui::SliderInt(label, &weight, 0, 20);
        }

        // Renders the effective probability beside a weight control.
        void render_probability_text(int weight, int total_weight)
        {
            ImGui::SameLine();
            ImGui::TextDisabled(
                "(%.1f%%)", probability_from_weights(weight, total_weight) * 100.0f
            );
        }
    }

    bool render_random_regex_button(
             const char* button_id,
             std::string& text,
             MacroCompletionState& macro_state,
             RandomRegexState& random_state,
             const app::operations::ExpressionFlavor flavor,
             const regex::generation::Config& finite_config,
             const regex::generation::OmegaConfig& omega_config
         )
    {
        ImGui::SameLine();
        const std::string label = std::string("⚄##") + button_id;
        bool changed = false;

        if (ImGui::Button(label.c_str()))
        {
            if (flavor == app::operations::ExpressionFlavor::OmegaRegex)
            {
                text = random_state.omega_generator.generate_string(omega_config);
            }
            else
            {
                text = random_state.generator.generate_string(finite_config);
            }

            reset_macro_completion(macro_state);
            changed = true;
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup(SettingsPopupId);
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(
                "Generate a random expression for the active word domain. "
                "Right-click to configure generation."
            );
        }

        return changed;
    }

    bool render_random_regex_config(regex::generation::Config& config)
    {
        ImGui::PushID(&config);

        bool changed = false;
        ImGui::TextUnformatted("Growth weights");

        const int growth_total =
            std::max(config.stop_weight, 0) + std::max(config.continue_weight, 0);

        changed |= render_weight_slider("Stop", config.stop_weight);
        render_probability_text(std::max(config.stop_weight, 0), growth_total);
        changed |= render_weight_slider("Continue", config.continue_weight);
        render_probability_text(std::max(config.continue_weight, 0), growth_total);

        ImGui::Separator();
        ImGui::TextUnformatted("Leaf weights");

        const int stopper_total = std::max(config.empty_set_weight, 0) +
                                  std::max(config.epsilon_weight, 0) +
                                  std::max(config.letter_weight, 0);

        changed |= render_weight_slider("∅", config.empty_set_weight);
        render_probability_text(std::max(config.empty_set_weight, 0), stopper_total);
        changed |= render_weight_slider("ε", config.epsilon_weight);
        render_probability_text(std::max(config.epsilon_weight, 0), stopper_total);
        changed |= render_weight_slider("a, b", config.letter_weight);
        render_probability_text(std::max(config.letter_weight, 0), stopper_total);

        ImGui::Separator();
        ImGui::TextUnformatted("Operator weights");

        const int continuer_total =
            std::max(config.kleene_star_weight, 0) + std::max(config.plus_weight, 0) +
            std::max(config.complement_weight, 0) + std::max(config.concatenation_weight, 0) +
            std::max(config.alternation_weight, 0) + std::max(config.intersection_weight, 0);

        changed |= render_weight_slider("Kleene star *", config.kleene_star_weight);
        render_probability_text(std::max(config.kleene_star_weight, 0), continuer_total);
        changed |= render_weight_slider("Plus +", config.plus_weight);
        render_probability_text(std::max(config.plus_weight, 0), continuer_total);
        changed |= render_weight_slider("Negation !", config.complement_weight);
        render_probability_text(std::max(config.complement_weight, 0), continuer_total);
        changed |= render_weight_slider("Concatenation", config.concatenation_weight);
        render_probability_text(std::max(config.concatenation_weight, 0), continuer_total);
        changed |= render_weight_slider("Alternation |", config.alternation_weight);
        render_probability_text(std::max(config.alternation_weight, 0), continuer_total);
        changed |= render_weight_slider("Intersection &", config.intersection_weight);
        render_probability_text(std::max(config.intersection_weight, 0), continuer_total);

        ImGui::Separator();
        ImGui::TextUnformatted("Limits");
        changed |= ImGui::SliderInt("Max depth", &config.max_depth, 1, 10);
        ImGui::Separator();

        if (ImGui::Button("Reset generator defaults"))
        {
            config = regex::generation::Config{};
            changed = true;
        }

        if (changed)
        {
            config = regex::generation::sanitize_config(config);
        }

        ImGui::PopID();
        return changed;
    }

    bool render_random_omega_regex_config(regex::generation::OmegaConfig& config)
    {
        ImGui::PushID(&config);

        bool changed = false;

        ImGui::TextUnformatted("Growth weights");

        const int growth_total =
            std::max(config.stop_weight, 0) + std::max(config.continue_weight, 0);

        changed |= render_weight_slider("Stop", config.stop_weight);
        render_probability_text(std::max(config.stop_weight, 0), growth_total);
        changed |= render_weight_slider("Continue", config.continue_weight);
        render_probability_text(std::max(config.continue_weight, 0), growth_total);

        ImGui::Separator();
        ImGui::TextUnformatted("Leaf weights");

        const int leaf_total = std::max(config.empty_set_weight, 0) +
                               std::max(config.universal_set_weight, 0) +
                               std::max(config.omega_power_weight, 0);

        changed |= render_weight_slider("∅", config.empty_set_weight);
        render_probability_text(std::max(config.empty_set_weight, 0), leaf_total);
        changed |= render_weight_slider("Σ^ω", config.universal_set_weight);
        render_probability_text(std::max(config.universal_set_weight, 0), leaf_total);
        changed |= render_weight_slider("S^ω", config.omega_power_weight);
        render_probability_text(std::max(config.omega_power_weight, 0), leaf_total);

        ImGui::Separator();
        ImGui::TextUnformatted("Operator weights");

        const int operator_total =
            std::max(config.prefix_concat_weight, 0) + std::max(config.alternation_weight, 0) +
            std::max(config.intersection_weight, 0) + std::max(config.complement_weight, 0);

        changed |= render_weight_slider("R S^ω", config.prefix_concat_weight);
        render_probability_text(std::max(config.prefix_concat_weight, 0), operator_total);
        changed |= render_weight_slider("Alternation |", config.alternation_weight);
        render_probability_text(std::max(config.alternation_weight, 0), operator_total);
        changed |= render_weight_slider("Intersection &", config.intersection_weight);
        render_probability_text(std::max(config.intersection_weight, 0), operator_total);
        changed |= render_weight_slider("Negation !", config.complement_weight);
        render_probability_text(std::max(config.complement_weight, 0), operator_total);

        ImGui::Separator();
        ImGui::TextUnformatted("Limits");
        changed |= ImGui::SliderInt("Max depth", &config.max_depth, 1, 10);

        ImGui::Separator();
        if (ImGui::TreeNode("Prefix finite generator R"))
        {
            ImGui::PushID("prefix_finite_generator");
            changed |= render_random_regex_config(config.prefix_config);
            ImGui::PopID();
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Period finite generator S"))
        {
            ImGui::PushID("period_finite_generator");
            ImGui::TextWrapped(
                "The period generator is used for S in S^ω and R S^ω. "
            );
            changed |= render_random_regex_config(config.period_config);
            ImGui::PopID();
            ImGui::TreePop();
        }

        ImGui::Separator();
        if (ImGui::Button("Reset omega generator defaults"))
        {
            config = regex::generation::OmegaConfig{};
            changed = true;
        }

        if (changed)
        {
            config = regex::generation::sanitize_omega_config(config);
        }

        ImGui::PopID();
        return changed;
    }

    void render_random_regex_settings_popup(
          regex::generation::Config& finite_config,
          regex::generation::OmegaConfig& omega_config
      )
    {
        ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, 0.0f), ImVec2(520.0f, 640.0f));
        if (!ImGui::BeginPopup(SettingsPopupId))
        {
            return;
        }

        ImGui::TextUnformatted("Random Expression Settings");
        ImGui::Separator();

        if (ImGui::BeginTabBar("##random_expression_settings_tabs"))
        {
            if (ImGui::BeginTabItem("Finite regex"))
            {
                (void)render_random_regex_config(finite_config);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Omega regex"))
            {
                (void)render_random_omega_regex_config(omega_config);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::EndPopup();
    }
}
