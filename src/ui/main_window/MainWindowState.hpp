/// @file
/// Defines persistent state owned by the main application window.
#pragma once

#include "app/settings/ApplicationSettings.hpp"
#include "ui/input/AlphabetInput.hpp"
#include "ui/input/RandomRegexControls.hpp"
#include "ui/input/RegexInput.hpp"
#include "ui/operations/OperationState.hpp"

namespace ui
{
    /// Owns all persistent state rendered by the main application window.
    struct MainWindowState
    {
        input::RegexInputState primary_input;
        input::RegexInputState secondary_input;
        input::AlphabetInputState alphabet;
        input::RandomRegexState random_regex;
        operations::OperationState operation;
        app::settings::ApplicationSettings settings;
        bool help_popup_requested = false;
        bool settings_visible = false;
    };
}
