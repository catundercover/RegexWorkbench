// Defines user-adjustable application settings and their valid ranges.
#pragma once

#include "regex/generation/RandomRegexGenerator.hpp"
#include "regex/generation/RandomOmegaRegexGenerator.hpp"
#include <chrono>
#include <cstdint>

namespace app::settings
{
    // Smallest operation deadline exposed by the settings UI.
    inline constexpr int MinimumOperationTimeoutSeconds = 1;
    // Largest operation deadline exposed by the settings UI.
    inline constexpr int MaximumOperationTimeoutSeconds = 60;
    // Smallest useful graph canvas height in screen pixels.
    inline constexpr float MinimumGraphCanvasHeight = 240.0F;
    // Largest graph canvas height exposed by the settings UI.
    inline constexpr float MaximumGraphCanvasHeight = 1000.0F;

    // Selects the application-wide color palette.
    enum class ColorTheme : std::uint8_t
    {
        Light,
        Dark,
        Count
    };

    // User-adjustable behavior that is safe to change at runtime.
    struct ApplicationSettings
    {
        int operation_timeout_seconds = 5;
        float graph_canvas_height = 500.0F;
        ColorTheme color_theme = ColorTheme::Light;
        regex::generation::Config random_regex;
        regex::generation::OmegaConfig random_omega_regex;
    };

    // Restores every setting to its documented default.
    [[nodiscard]] ApplicationSettings defaults();

    // Clamps settings to supported ranges and repairs unusable generator weights.
    void normalize(ApplicationSettings& settings);

    // Converts the configured timeout to the controller's duration type.
    [[nodiscard]] std::chrono::milliseconds operation_timeout(const ApplicationSettings& settings);
}
