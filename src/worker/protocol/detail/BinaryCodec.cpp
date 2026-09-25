// Implements bounded binary primitive encoding and decoding.
#include "worker/protocol/detail/BinaryCodec.hpp"

#include "worker/protocol/ProtocolLimits.hpp"

#include <bit>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace worker::protocol::detail
{
    void Writer::write_u8(const std::uint8_t value)
    {
        data_.push_back(value);
    }

    void Writer::write_bool(const bool value)
    {
        write_u8(value ? 1U : 0U);
    }

    void Writer::write_u32(const std::uint32_t value)
    {
        for (unsigned int shift = 0; shift < 32; shift += 8)
        {
            write_u8(static_cast<std::uint8_t>((value >> shift) & 0xFFU));
        }
    }

    void Writer::write_float(const float value)
    {
        write_u32(std::bit_cast<std::uint32_t>(value));
    }

    void Writer::write_string(const std::string& value)
    {
        if (value.size() > std::numeric_limits<std::uint32_t>::max())
        {
            throw std::length_error("Protocol string is too large.");
        }
        write_u32(static_cast<std::uint32_t>(value.size()));
        data_.insert(data_.end(), value.begin(), value.end());
    }

    void Writer::write_count(const std::size_t value)
    {
        if (value > std::numeric_limits<std::uint32_t>::max())
        {
            throw std::length_error("Protocol collection is too large.");
        }
        write_u32(static_cast<std::uint32_t>(value));
    }

    Payload Writer::finish()
    {
        if (data_.size() > limits::MaxPayloadBytes)
        {
            throw std::length_error("Protocol payload exceeds the configured limit.");
        }
        return std::move(data_);
    }

    Reader::Reader(const std::span<const std::uint8_t> data) : data_(data)
    {
        if (data.size() > limits::MaxPayloadBytes)
        {
            throw std::runtime_error("Protocol payload exceeds the configured limit.");
        }
    }

    std::uint8_t Reader::read_u8()
    {
        require(1);
        return data_[position_++];
    }

    bool Reader::read_bool()
    {
        const std::uint8_t value = read_u8();
        if (value > 1)
        {
            throw std::runtime_error("Protocol boolean is invalid.");
        }
        return value == 1;
    }

    std::uint32_t Reader::read_u32()
    {
        require(4);
        std::uint32_t value = 0;
        for (unsigned int shift = 0; shift < 32; shift += 8)
        {
            value |= static_cast<std::uint32_t>(data_[position_++]) << shift;
        }
        return value;
    }

    float Reader::read_float()
    {
        const float value = std::bit_cast<float>(read_u32());
        if (!std::isfinite(value))
        {
            throw std::runtime_error("Protocol coordinate is not finite.");
        }
        return value;
    }

    std::string Reader::read_string()
    {
        const std::size_t size = read_u32();
        require(size);
        const char* begin = reinterpret_cast<const char*>(data_.data() + position_);
        position_ += size;
        return std::string(begin, size);
    }

    std::size_t Reader::read_count()
    {
        const std::size_t count = read_u32();
        if (count > limits::MaxPayloadBytes)
        {
            throw std::runtime_error("Protocol collection count is invalid.");
        }
        return count;
    }

    void Reader::require_finished() const
    {
        if (position_ != data_.size())
        {
            throw std::runtime_error("Protocol payload has trailing data.");
        }
    }

    void Reader::require(const std::size_t size) const
    {
        if (size > data_.size() - position_)
        {
            throw std::runtime_error("Protocol payload is truncated.");
        }
    }
}
