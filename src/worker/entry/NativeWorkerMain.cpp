// Implements framed standard-I/O entry for the native operation worker.
#include "worker/OperationWorker.hpp"
#include "worker/protocol/PayloadFrame.hpp"
#include "worker/protocol/ProtocolLimits.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>

// Processes one framed request from standard input and writes one framed result.
int main()
{
    worker::protocol::PayloadFrameHeader header{};
    if (!std::cin.read(
            reinterpret_cast<char*>(header.data()), static_cast<std::streamsize>(header.size())
        ))
    {
        return 1;
    }

    const std::size_t request_size = worker::protocol::decode_payload_size(header);
    if (request_size > worker::protocol::limits::MaxPayloadBytes)
    {
        return 1;
    }

    worker::protocol::Payload request(request_size);
    if (!std::cin.read(
            reinterpret_cast<char*>(request.data()), static_cast<std::streamsize>(request.size())
        ))
    {
        return 1;
    }

    const worker::protocol::Payload response = worker::process_operation(request);
    const worker::protocol::PayloadFrameHeader response_header =
        worker::protocol::encode_payload_size(response.size());
    if (!std::cout.write(
            reinterpret_cast<const char*>(response_header.data()),
            static_cast<std::streamsize>(response_header.size())
        ) ||
        !std::cout.write(
            reinterpret_cast<const char*>(response.data()),
            static_cast<std::streamsize>(response.size())
        ))
    {
        return 1;
    }

    std::cout.flush();
    return std::cout.good() ? 0 : 1;
}
