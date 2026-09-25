// Implements defaults and normalization for application settings.
#include "app/settings/ApplicationSettings.hpp"

#include <algorithm>
#include <cmath>

namespace app::settings
{
    ApplicationSettings defaults()
    {
        return ApplicationSettings{};
    }

    void normalize(ApplicationSettings& settings)
    {
        settings.operation_timeout_seconds = std::clamp(
            settings.operation_timeout_seconds,
            MinimumOperationTimeoutSeconds,
            MaximumOperationTimeoutSeconds
        );
        if (!std::isfinite(settings.graph_canvas_height))
        {
            settings.graph_canvas_height = defaults().graph_canvas_height;
        }
        else
        {
            settings.graph_canvas_height = std::clamp(
                settings.graph_canvas_height, MinimumGraphCanvasHeight, MaximumGraphCanvasHeight
            );
        }

        switch (settings.color_theme)
        {
        case ColorTheme::Light:
        case ColorTheme::Dark:
            break;
        case ColorTheme::Count:
        default:
            settings.color_theme = defaults().color_theme;
            break;
        }
        settings.random_regex = regex::generation::sanitize_config(settings.random_regex);
        settings.random_omega_regex = regex::generation::sanitize_omega_config(settings.random_omega_regex);
    }

    std::chrono::milliseconds operation_timeout(const ApplicationSettings& settings)
    {
        return std::chrono::seconds(settings.operation_timeout_seconds);
    }
}
