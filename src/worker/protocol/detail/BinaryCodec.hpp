// Defines checked primitive readers and writers for the binary protocol.
#pragma once

#include "worker/protocol/OperationProtocol.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <utility>

namespace worker::protocol::detail
{
    // Writes the primitive, length-prefixed values used by the operation wire format.
    class Writer
    {
    public:
        // Appends one unsigned byte.
        void write_u8(std::uint8_t value);
        // Appends a canonical zero-or-one boolean byte.
        void write_bool(bool value);
        // Appends a little-endian 32-bit unsigned integer.
        void write_u32(std::uint32_t value);
        // Appends a finite IEEE-754 single-precision value.
        void write_float(float value);
        // Appends a length-prefixed byte string.
        void write_string(const std::string& value);
        // Appends a collection count after checking its wire-width limit.
        void write_count(std::size_t value);

        // Appends a presence flag and, when present, a caller-encoded value.
        template <typename Value, typename WriteValue>
        void write_optional(const std::optional<Value>& value, WriteValue&& write_value)
        {
            write_bool(value.has_value());
            if (value)
            {
                std::forward<WriteValue>(write_value)(*value);
            }
        }

        // Validates the total size and transfers ownership of the payload.
        [[nodiscard]] Payload finish();

    private:
        Payload data_;
    };

    // Reads primitive values while enforcing payload bounds and value validity.
    class Reader
    {
    public:
        // Creates a reader and rejects payloads above the transport ceiling.
        explicit Reader(std::span<const std::uint8_t> data);

        // Reads one unsigned byte.
        [[nodiscard]] std::uint8_t read_u8();
        // Reads and validates a canonical boolean byte.
        [[nodiscard]] bool read_bool();
        // Reads a little-endian 32-bit unsigned integer.
        [[nodiscard]] std::uint32_t read_u32();
        // Reads and validates a finite single-precision value.
        [[nodiscard]] float read_float();
        // Reads a bounded length-prefixed byte string.
        [[nodiscard]] std::string read_string();
        // Reads a collection count representable on the current platform.
        [[nodiscard]] std::size_t read_count();

        // Reads a presence flag and, when present, a caller-decoded value.
        template <typename ReadValue>
        [[nodiscard]] auto read_optional(ReadValue&& read_value)
        {
            using Value = std::invoke_result_t<ReadValue>;
            if (!read_bool())
            {
                return std::optional<Value>{};
            }
            return std::optional<Value>{std::forward<ReadValue>(read_value)()};
        }

        // Rejects trailing bytes after a complete message has been decoded.
        void require_finished() const;

    private:
        // Ensures the requested byte count remains within the unread payload.
        void require(std::size_t size) const;

        std::span<const std::uint8_t> data_;
        std::size_t position_ = 0;
    };
}
