/// @file
/// Declares the application settings window.
#pragma once

namespace app::settings
{
    struct ApplicationSettings;
}

namespace ui::settings
{
    /// Renders and optionally focuses the settings window; reports effective changes.
    [[nodiscard]] bool render_settings_page(
        app::settings::ApplicationSettings& settings, bool& visible, bool focus_requested
    );
}
