// Implements deadlines, cancellation, and result ownership for operations.
#include "app/operations/OperationController.hpp"

#include "app/operations/OperationRunner.hpp"

#include <stdexcept>
#include <utility>

namespace app::operations
{
    OperationController::OperationController()
        : OperationController(make_platform_operation_runner())
    {
    }

    OperationController::OperationController(
        std::unique_ptr<OperationRunner> runner, const std::chrono::steady_clock::duration timeout
    )
        : runner_(std::move(runner)), timeout_(timeout)
    {
        if (!runner_)
        {
            throw std::invalid_argument("Operation controller requires a runner.");
        }
    }

    OperationController::~OperationController()
    {
        runner_->cancel();
    }

    bool OperationController::start(OperationRequest request)
    {
        if (runner_->is_running())
        {
            return false;
        }

        result_.reset();
        if (!runner_->start(std::move(request)))
        {
            return false;
        }
        started_at_ = std::chrono::steady_clock::now();
        return true;
    }

    void OperationController::poll()
    {
        if (std::optional<OperationResult> result = runner_->poll())
        {
            result_ = std::move(*result);
            return;
        }

        if (!runner_->is_running())
        {
            return;
        }

        if (std::chrono::steady_clock::now() - started_at_ < timeout_)
        {
            return;
        }

        runner_->cancel();
        result_ =
            OperationFailure{"The operation exceeded the configured time limit and was cancelled."};
    }

    void OperationController::cancel()
    {
        runner_->cancel();
        result_.reset();
    }

    void
    OperationController::set_timeout(const std::chrono::steady_clock::duration timeout) noexcept
    {
        timeout_ = timeout;
    }

    std::chrono::steady_clock::duration OperationController::timeout() const noexcept
    {
        return timeout_;
    }

    bool OperationController::is_running() const noexcept
    {
        return runner_->is_running();
    }

    std::optional<OperationResult> OperationController::take_result()
    {
        std::optional<OperationResult> result = std::move(result_);
        result_.reset();
        return result;
    }
}
