/// @file
/// Declares small widgets shared by result views.
#pragma once

#include <chrono>
#include <string_view>

namespace ui::results
{
    /// Delay before activity feedback is shown, avoiding flashes for fast operations.
    inline constexpr auto ActivityIndicatorDelay = std::chrono::milliseconds(250);

    /// Returns whether an operation has run long enough to warrant activity feedback.
    [[nodiscard]] inline bool activity_delay_elapsed(
        const std::chrono::steady_clock::time_point started_at,
        const std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now()
    ) noexcept
    {
        return current_time - started_at >= ActivityIndicatorDelay;
    }

    /// Renders an operation failure with semantic coloring.
    void render_error_message(std::string_view message);

    /// Renders the shared spinner, label, and optional cancellation control.
    [[nodiscard]] bool render_activity(std::string_view label, const char* cancel_label = nullptr);
}
