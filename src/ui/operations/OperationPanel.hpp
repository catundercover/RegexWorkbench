/// @file
/// Declares the controls for selecting and starting regex operations.
#pragma once

namespace regex::generation
{
    struct Config;
    struct OmegaConfig;
}

namespace ui::input
{
    struct AlphabetInputState;
    struct RandomRegexState;
    struct RegexInputState;
}

namespace ui::operations
{
    class OperationCoordinator;
    struct OperationState;

    /// Bundles shared input state required by the operation panel.
    struct OperationPanelInputs
    {
        input::RegexInputState& primary;
        input::RegexInputState& secondary;
        input::RandomRegexState& random_regex;
        regex::generation::Config& random_regex_config;
        regex::generation::OmegaConfig& random_omega_regex_config;
        input::AlphabetInputState& alphabet;
    };

    /// Renders the mode selector and controls for the active regex operation.
    void render_operation_panel(
        OperationState& state, const OperationPanelInputs& inputs, OperationCoordinator& coordinator
    );
}
