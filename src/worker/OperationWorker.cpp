// Implements one fault-contained worker request/response cycle.
#include "worker/OperationWorker.hpp"

#include "app/operations/OperationExecutor.hpp"

#include <exception>
#include <string>
#include <utility>
#include <variant>

namespace worker
{
    protocol::Payload process_operation(const std::span<const std::uint8_t> request_payload)
    {
        try
        {
            auto decoded = protocol::decode_request(request_payload);
            if (const auto* error = std::get_if<protocol::ProtocolError>(&decoded))
            {
                return protocol::encode_result(
                    app::operations::OperationFailure{
                        "The operation worker received an invalid request: " + error->message
                    }
                );
            }

            app::operations::OperationResult result = app::operations::execute(
                std::get<app::operations::OperationRequest>(std::move(decoded))
            );
            return protocol::encode_result(result);
        }
        catch (const std::exception& exception)
        {
            return protocol::encode_result(
                app::operations::OperationFailure{
                    "The operation worker failed: " + std::string(exception.what())
                }
            );
        }
        catch (...)
        {
            return protocol::encode_result(
                app::operations::OperationFailure{"The operation worker failed unexpectedly."}
            );
        }
    }
}
