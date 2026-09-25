// Implements fixed-width little-endian payload frame headers.
#include "worker/protocol/PayloadFrame.hpp"

#include <limits>
#include <stdexcept>

namespace worker::protocol
{
    PayloadFrameHeader encode_payload_size(const std::size_t size)
    {
        if (size > std::numeric_limits<std::uint32_t>::max())
        {
            throw std::length_error("Protocol payload cannot be represented by its frame.");
        }

        const auto value = static_cast<std::uint32_t>(size);
        return {
            static_cast<std::uint8_t>(value & 0xFFU),
            static_cast<std::uint8_t>((value >> 8U) & 0xFFU),
            static_cast<std::uint8_t>((value >> 16U) & 0xFFU),
            static_cast<std::uint8_t>((value >> 24U) & 0xFFU)
        };
    }

    std::uint32_t
    decode_payload_size(const std::span<const std::uint8_t, PayloadFrameHeaderSize> header)
    {
        return static_cast<std::uint32_t>(header[0]) | static_cast<std::uint32_t>(header[1]) << 8U |
               static_cast<std::uint32_t>(header[2]) << 16U |
               static_cast<std::uint32_t>(header[3]) << 24U;
    }
}
