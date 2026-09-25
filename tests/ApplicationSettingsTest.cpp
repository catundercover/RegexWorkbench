// Verifies application-setting defaults and normalization.
#include "app/settings/ApplicationSettings.hpp"

#include <chrono>
#include <iostream>
#include <limits>
#include <string>

namespace
{
    // Reports a failed assertion and returns its condition.
    bool check(const bool condition, const std::string& message)
    {
        if (!condition)
        {
            std::cerr << message << '\n';
        }
        return condition;
    }
}

// Runs application-setting default and normalization checks.
int main()
{
    app::settings::ApplicationSettings settings = app::settings::defaults();
    if (!check(settings.operation_timeout_seconds == 5, "The default timeout changed.") ||
        !check(settings.graph_canvas_height == 500.0F, "The default graph height changed.") ||
        !check(
            settings.color_theme == app::settings::ColorTheme::Light,
            "The default color theme changed."
        ) ||
        !check(
            settings.random_regex.stop_weight == 1 && settings.random_regex.continue_weight == 3,
            "The default generator growth probabilities changed."
        ) ||
        !check(
            app::settings::operation_timeout(settings) == std::chrono::seconds(5),
            "The timeout duration does not match the setting."
        ))
    {
        return 1;
    }

    settings.operation_timeout_seconds = -10;
    settings.graph_canvas_height = std::numeric_limits<float>::quiet_NaN();
    settings.color_theme = app::settings::ColorTheme::Count;
    settings.random_regex.stop_weight = -1;
    settings.random_regex.continue_weight = -2;
    settings.random_regex.empty_set_weight = 0;
    settings.random_regex.epsilon_weight = 0;
    settings.random_regex.letter_weight = 0;
    settings.random_regex.max_depth = 100;
    app::settings::normalize(settings);

    if (!check(
            settings.operation_timeout_seconds == app::settings::MinimumOperationTimeoutSeconds,
            "The timeout was not clamped to its minimum."
        ) ||
        !check(
            settings.graph_canvas_height == 500.0F, "A non-finite graph height was not reset."
        ) ||
        !check(
            settings.color_theme == app::settings::ColorTheme::Light,
            "An invalid color theme was not reset."
        ) ||
        !check(
            settings.random_regex.stop_weight == 1 && settings.random_regex.continue_weight == 0,
            "Invalid growth weights were not repaired."
        ) ||
        !check(
            settings.random_regex.letter_weight == 1, "Invalid leaf weights were not repaired."
        ) ||
        !check(settings.random_regex.max_depth == 10, "Generator depth was not clamped."))
    {
        return 1;
    }

    settings.operation_timeout_seconds = 1000;
    settings.graph_canvas_height = 10000.0F;
    settings.color_theme = app::settings::ColorTheme::Dark;
    app::settings::normalize(settings);
    if (!check(
            settings.operation_timeout_seconds == app::settings::MaximumOperationTimeoutSeconds,
            "The timeout was not clamped to its maximum."
        ) ||
        !check(
            settings.graph_canvas_height == app::settings::MaximumGraphCanvasHeight,
            "The graph height was not clamped to its maximum."
        ) ||
        !check(
            settings.color_theme == app::settings::ColorTheme::Dark,
            "A valid dark theme was not preserved."
        ))
    {
        return 1;
    }

    return 0;
}
