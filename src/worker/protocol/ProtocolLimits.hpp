// Defines hard safety limits for worker protocol payloads.
#pragma once

#include <cstddef>

namespace worker::protocol::limits
{
    // Hard transport ceiling; intentionally not exposed as an application setting.
    inline constexpr std::size_t MaxPayloadBytes = std::size_t{16} * 1024 * 1024;
}
