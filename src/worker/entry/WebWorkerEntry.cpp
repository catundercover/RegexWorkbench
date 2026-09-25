// Exposes operation processing to the Emscripten Web Worker host.
#include "worker/OperationWorker.hpp"

#include <cstddef>
#include <cstdint>
#include <emscripten.h>
#include <span>

// Processes one Emscripten worker request and responds with its serialized result.
extern "C" EMSCRIPTEN_KEEPALIVE void run_operation(char* data, const int size)
{
    if (data == nullptr || size < 0)
    {
        emscripten_worker_respond(nullptr, 0);
        return;
    }

    const auto* bytes = reinterpret_cast<const std::uint8_t*>(data);
    worker::protocol::Payload response =
        worker::process_operation(std::span(bytes, static_cast<std::size_t>(size)));
    emscripten_worker_respond(
        reinterpret_cast<char*>(response.data()), static_cast<int>(response.size())
    );
}
