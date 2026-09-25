// Defines the fixed-width size prefix used by native worker messages.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace worker::protocol
{
    // Number of bytes in a framed payload length.
    constexpr std::size_t PayloadFrameHeaderSize = 4;
    // Fixed-width byte representation of a framed payload length.
    using PayloadFrameHeader = std::array<std::uint8_t, PayloadFrameHeaderSize>;

    // Encodes a payload length as the protocol's little-endian 32-bit frame header.
    [[nodiscard]] PayloadFrameHeader encode_payload_size(std::size_t size);
    // Decodes the protocol's little-endian 32-bit payload length.
    [[nodiscard]] std::uint32_t
    decode_payload_size(std::span<const std::uint8_t, PayloadFrameHeaderSize> header);
}
