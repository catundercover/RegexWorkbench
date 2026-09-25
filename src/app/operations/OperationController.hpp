// Declares lifecycle control for isolated application operations.
#pragma once

#include "app/operations/OperationLimits.hpp"
#include "app/operations/OperationModels.hpp"

#include <chrono>
#include <memory>
#include <optional>

namespace app::operations
{
    class OperationRunner;

    // Owns the single active operation, its deadline, and its pending result.
    class OperationController
    {
    public:
        // Creates a controller backed by the current platform runner.
        OperationController();
        /// Creates a controller with an injected runner and execution timeout.
        explicit OperationController(
            std::unique_ptr<OperationRunner> runner,
            std::chrono::steady_clock::duration timeout = limits::ExecutionTimeout
        );
        // Cancels an active operation before releasing the runner.
        ~OperationController();

        // Controllers exclusively own their runner and cannot be copied.
        OperationController(const OperationController&) = delete;
        // Controllers exclusively own their runner and cannot be copied.
        OperationController& operator=(const OperationController&) = delete;

        // Starts a request if no operation or unread result is pending.
        bool start(OperationRequest request);
        // Collects a completed result or enforces the active deadline.
        void poll();
        // Cancels the active request and clears its pending result.
        void cancel();
        // Replaces the deadline used for subsequently started operations.
        void set_timeout(std::chrono::steady_clock::duration timeout) noexcept;
        // Returns the configured operation timeout.
        [[nodiscard]] std::chrono::steady_clock::duration timeout() const noexcept;
        // Returns whether the runner currently owns an active request.
        [[nodiscard]] bool is_running() const noexcept;
        // Removes and returns the pending result, if one is available.
        [[nodiscard]] std::optional<OperationResult> take_result();

    private:
        std::unique_ptr<OperationRunner> runner_;
        std::chrono::steady_clock::duration timeout_;
        std::chrono::steady_clock::time_point started_at_;
        std::optional<OperationResult> result_;
    };
}
