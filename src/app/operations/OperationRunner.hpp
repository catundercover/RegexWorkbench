// Declares the asynchronous boundary used to run expensive operations.
#pragma once

#include "app/operations/OperationModels.hpp"

#include <memory>
#include <optional>

namespace app::operations
{
    // Runs at most one operation outside the UI event loop.
    class OperationRunner
    {
    public:
        // Allows platform runners to release their worker resources polymorphically.
        virtual ~OperationRunner() = default;

        // Starts a request and returns false when another request is active.
        virtual bool start(OperationRequest request) = 0;
        // Returns and consumes a completed result without blocking.
        [[nodiscard]] virtual std::optional<OperationResult> poll() = 0;
        // Terminates the active worker operation, if any.
        virtual void cancel() = 0;
        // Returns whether the runner currently owns an active operation.
        [[nodiscard]] virtual bool is_running() const noexcept = 0;
    };

    // Creates the isolated worker implementation for the current platform.
    [[nodiscard]] std::unique_ptr<OperationRunner> make_platform_operation_runner();
}
