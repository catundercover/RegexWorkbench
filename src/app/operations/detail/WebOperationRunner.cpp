// Implements isolated browser operations through a reusable Web Worker.
#include "app/operations/OperationRunner.hpp"
#include "app/operations/detail/PlatformRunnerFactory.hpp"
#include "worker/protocol/OperationProtocol.hpp"

#include <cstddef>
#include <cstdint>
#include <emscripten.h>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <variant>

namespace app::operations::detail
{
    namespace
    {
        // Wraps a malformed worker response in the operation result model.
        [[nodiscard]] OperationResult protocol_failure(const std::string& detail)
        {
            return OperationFailure{"The operation worker returned invalid data: " + detail};
        }

        // Runs one operation at a time in a reusable Emscripten Web Worker.
        class WebOperationRunner final : public OperationRunner
        {
        public:
            // Releases the browser worker when the runner is destroyed.
            ~WebOperationRunner() override
            {
                destroy_worker();
            }

            // Serializes and posts a request when the runner is idle.
            bool start(OperationRequest request) override
            {
                if (is_running())
                {
                    return false;
                }

                try
                {
                    worker::protocol::Payload payload = worker::protocol::encode_request(request);
                    if (!ensure_worker())
                    {
                        return false;
                    }
                    emscripten_call_worker(
                        worker_,
                        "run_operation",
                        reinterpret_cast<char*>(payload.data()),
                        static_cast<int>(payload.size()),
                        &WebOperationRunner::receive,
                        this
                    );
                    running_ = true;
                    return true;
                }
                catch (...)
                {
                    cancel();
                    return false;
                }
            }

            // Transfers a callback-produced result to the UI thread.
            std::optional<OperationResult> poll() override
            {
                std::optional<OperationResult> result = std::move(result_);
                result_.reset();
                return result;
            }

            // Terminates an active worker so non-cooperative work stops reliably.
            void cancel() override
            {
                if (running_)
                {
                    destroy_worker();
                }
                running_ = false;
                result_.reset();
            }

            // Returns whether a posted request is awaiting its callback.
            [[nodiscard]] bool is_running() const noexcept override
            {
                return running_;
            }

        private:
            // Decodes the response delivered by Emscripten's worker callback.
            static void receive(char* data, const int size, void* context)
            {
                auto& self = *static_cast<WebOperationRunner*>(context);
                if (!self.running_)
                {
                    return;
                }

                if (data == nullptr || size < 0)
                {
                    self.result_ = protocol_failure("the response is empty.");
                    self.destroy_worker();
                }
                else
                {
                    const auto* bytes = reinterpret_cast<const std::uint8_t*>(data);
                    auto decoded = worker::protocol::decode_result(
                        std::span(bytes, static_cast<std::size_t>(size))
                    );
                    if (const auto* error = std::get_if<worker::protocol::ProtocolError>(&decoded))
                    {
                        self.result_ = protocol_failure(error->message);
                        // A protocol error makes worker reuse unsafe; recreate it next time.
                        self.destroy_worker();
                    }
                    else
                    {
                        self.result_ = std::get<OperationResult>(std::move(decoded));
                    }
                }

                self.running_ = false;
            }

            // Reuses a healthy worker or creates one on demand.
            bool ensure_worker()
            {
                if (worker_ > 0)
                {
                    return true;
                }
                worker_ = emscripten_create_worker("operation-worker.js");
                return worker_ > 0;
            }

            // Destroys the browser worker and clears its handle.
            void destroy_worker()
            {
                if (worker_ > 0)
                {
                    emscripten_destroy_worker(worker_);
                    worker_ = 0;
                }
            }

            worker_handle worker_ = 0;
            bool running_ = false;
            std::optional<OperationResult> result_;
        };
    }

    std::unique_ptr<OperationRunner> make_platform_runner()
    {
        return std::make_unique<WebOperationRunner>();
    }
}
