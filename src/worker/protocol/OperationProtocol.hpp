// Defines serialization APIs for operation requests and results.
#pragma once

#include "app/operations/OperationModels.hpp"

#include <cstdint>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace worker::protocol
{
    // Serialized operation message transported across a worker boundary.
    using Payload = std::vector<std::uint8_t>;

    // Reports malformed or unsupported protocol data.
    struct ProtocolError
    {
        std::string message;
    };

    // Contains either a decoded value or a protocol diagnostic.
    template <typename Value>
    using DecodeResult = std::variant<Value, ProtocolError>;

    // Serializes an operation request into the binary wire format.
    [[nodiscard]] Payload encode_request(const app::operations::OperationRequest& request);
    // Decodes and validates one complete operation-request payload.
    [[nodiscard]] DecodeResult<app::operations::OperationRequest>
    decode_request(std::span<const std::uint8_t> payload);

    // Serializes an operation result into the binary wire format.
    [[nodiscard]] Payload encode_result(const app::operations::OperationResult& result);
    // Decodes and validates one complete operation-result payload.
    [[nodiscard]] DecodeResult<app::operations::OperationResult>
    decode_result(std::span<const std::uint8_t> payload);
}
