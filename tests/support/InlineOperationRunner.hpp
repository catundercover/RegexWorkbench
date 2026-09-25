// Defines a deterministic in-process operation runner for unit tests.
#pragma once

#include "app/operations/OperationExecutor.hpp"
#include "app/operations/OperationRunner.hpp"

#include <optional>
#include <utility>

namespace test_support
{
    // Executes synchronously while preserving the asynchronous runner contract for unit tests.
    class InlineOperationRunner final : public app::operations::OperationRunner
    {
    public:
        // Executes one request immediately unless a prior result is pending.
        bool start(app::operations::OperationRequest request) override
        {
            if (running_)
            {
                return false;
            }
            result_ = app::operations::execute(std::move(request));
            running_ = true;
            return true;
        }

        // Returns the synchronously computed result exactly once.
        [[nodiscard]] std::optional<app::operations::OperationResult> poll() override
        {
            if (!running_)
            {
                return std::nullopt;
            }
            running_ = false;
            std::optional<app::operations::OperationResult> result = std::move(result_);
            result_.reset();
            return result;
        }

        // Discards a pending result and restores the idle state.
        void cancel() override
        {
            running_ = false;
            result_.reset();
        }

        // Returns whether a computed result is waiting to be polled.
        [[nodiscard]] bool is_running() const noexcept override
        {
            return running_;
        }

    private:
        bool running_ = false;
        std::optional<app::operations::OperationResult> result_;
    };
}
