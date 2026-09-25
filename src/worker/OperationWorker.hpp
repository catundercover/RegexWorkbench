// Declares the isolated worker's single-request processing entry point.
#pragma once

#include "worker/protocol/OperationProtocol.hpp"

#include <span>

namespace worker
{
    // Decodes, executes, and encodes exactly one isolated operation.
    [[nodiscard]] protocol::Payload
    process_operation(std::span<const std::uint8_t> request_payload);
}
