// Implements versioned binary serialization of operation messages.
#include "worker/protocol/OperationProtocol.hpp"

#include "worker/protocol/detail/BinaryCodec.hpp"
#include "worker/protocol/detail/GraphCodec.hpp"

#include <cstdint>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace worker::protocol
{
    namespace
    {
        // Fixed protocol signature encoded as the little-endian bytes `SLUV`.
        constexpr std::uint32_t Magic = 0x56554C53U; // "SLUV" in little-endian order.
        // Current incompatible wire-format version.
        constexpr std::uint8_t Version = 4;

        // Distinguishes request frames from result frames.
        enum class PayloadKind : std::uint8_t
        {
            Request,
            Result
        };

        // Writes the common protocol signature, version, and payload kind.
        void write_header(detail::Writer& writer, const PayloadKind kind)
        {
            writer.write_u32(Magic);
            writer.write_u8(Version);
            writer.write_u8(static_cast<std::uint8_t>(kind));
        }

        // Reads and validates the common protocol header.
        void require_header(detail::Reader& reader, const PayloadKind expected_kind)
        {
            if (reader.read_u32() != Magic)
            {
                throw std::runtime_error("Protocol magic is invalid.");
            }
            if (reader.read_u8() != Version)
            {
                throw std::runtime_error("Protocol version is unsupported.");
            }
            if (reader.read_u8() != static_cast<std::uint8_t>(expected_kind))
            {
                throw std::runtime_error("Protocol payload kind is invalid.");
            }
        }

        // Reads and validates the language universe attached to an operation.
        app::operations::ExpressionFlavor read_expression_flavor(detail::Reader& reader)
        {
            const std::uint8_t encoded = reader.read_u8();
            if (encoded > static_cast<std::uint8_t>(app::operations::ExpressionFlavor::OmegaRegex))
            {
                throw std::runtime_error("Protocol expression flavor is invalid.");
            }
            return static_cast<app::operations::ExpressionFlavor>(encoded);
        }

        // Converts decoder exceptions into a typed protocol error.
        template <typename Value, typename Decode>
        [[nodiscard]] DecodeResult<Value>
        decode(const std::span<const std::uint8_t> payload, Decode&& decode_value)
        {
            try
            {
                detail::Reader reader(payload);
                Value value = std::forward<Decode>(decode_value)(reader);
                reader.require_finished();
                return value;
            }
            catch (const std::exception& exception)
            {
                return ProtocolError{exception.what()};
            }
        }
    }

    Payload encode_request(const app::operations::OperationRequest& request)
    {
        detail::Writer writer;
        write_header(writer, PayloadKind::Request);
        std::visit(
            [&writer](const auto& value)
            {
                using Request = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<Request, app::operations::TranslateRequest>)
                {
                    writer.write_u8(0);
                    writer.write_string(value.regex);
                    writer.write_u8(static_cast<std::uint8_t>(value.flavor));
                    writer.write_u8(static_cast<std::uint8_t>(value.mode));
                    writer.write_string(value.extra_alphabet);
                }
                else if constexpr (std::is_same_v<Request, app::operations::RewriteRequest>)
                {
                    writer.write_u8(1);
                    writer.write_string(value.regex);
                    writer.write_u8(static_cast<std::uint8_t>(value.flavor));
                    writer.write_string(value.extra_alphabet);
                    writer.write_bool(value.remove_complement);
                    writer.write_bool(value.remove_intersection);
                    writer.write_bool(value.remove_power);
                    writer.write_bool(value.remove_plus);
                    writer.write_bool(value.remove_any_symbol);
                }
                else
                {
                    writer.write_u8(2);
                    writer.write_u8(static_cast<std::uint8_t>(value.flavor));
                    writer.write_string(value.left_regex);
                    writer.write_string(value.right_regex);
                    writer.write_string(value.extra_alphabet);
                }
            },
            request
        );
        return writer.finish();
    }

    DecodeResult<app::operations::OperationRequest>
    decode_request(const std::span<const std::uint8_t> payload)
    {
        return decode<app::operations::OperationRequest>(
            payload,
            [](detail::Reader& reader) -> app::operations::OperationRequest
            {
                require_header(reader, PayloadKind::Request);
                switch (reader.read_u8())
                {
                case 0:
                {
                    app::operations::TranslateRequest request;
                    request.regex = reader.read_string();
                    request.flavor = read_expression_flavor(reader);

                    const std::uint8_t mode = reader.read_u8();
                    if (mode > static_cast<std::uint8_t>(app::operations::TranslateMode::Dfa))
                    {
                        throw std::runtime_error("Protocol translation mode is invalid.");
                    }
                    request.mode = static_cast<app::operations::TranslateMode>(mode);
                    request.extra_alphabet = reader.read_string();
                    return request;
                }
                case 1:
                {
                    app::operations::RewriteRequest request;
                    request.regex = reader.read_string();
                    request.flavor = read_expression_flavor(reader);
                    request.extra_alphabet = reader.read_string();
                    request.remove_complement = reader.read_bool();
                    request.remove_intersection = reader.read_bool();
                    request.remove_power = reader.read_bool();
                    request.remove_plus = reader.read_bool();
                    request.remove_any_symbol = reader.read_bool();
                    return request;
                }
                case 2:
                {
                    app::operations::CompareRequest request;
                    request.flavor = read_expression_flavor(reader);
                    request.left_regex = reader.read_string();
                    request.right_regex = reader.read_string();
                    request.extra_alphabet = reader.read_string();
                    return request;
                }
                default:
                    throw std::runtime_error("Protocol request type is invalid.");
                }
            }
        );
    }

    Payload encode_result(const app::operations::OperationResult& result)
    {
        detail::Writer writer;
        write_header(writer, PayloadKind::Result);
        std::visit(
            [&writer](const auto& value)
            {
                using Result = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<Result, app::operations::OperationFailure>)
                {
                    writer.write_u8(0);
                    writer.write_string(value.message);
                }
                else if constexpr (std::is_same_v<Result, app::operations::TranslateResult>)
                {
                    writer.write_u8(1);
                    detail::write_layout(writer, value.graph_layout);
                }
                else if constexpr (std::is_same_v<Result, app::operations::RewriteResult>)
                {
                    writer.write_u8(2);
                    writer.write_string(value.rewritten_regex);
                }
                else
                {
                    writer.write_u8(3);
                    std::visit(
                        [&writer](const auto& comparison)
                        {
                            using Comparison = std::decay_t<decltype(comparison)>;

                            if constexpr (
                                std::is_same_v<Comparison, automata::analysis::RegexComparison>
                            )
                            {
                                writer.write_u8(0);
                                detail::write_comparison(writer, comparison);
                            }
                            else
                            {
                                writer.write_u8(1);
                                detail::write_omega_comparison(writer, comparison);
                            }
                        },
                        value.comparison
                    );
                }
            },
            result
        );
        return writer.finish();
    }

    DecodeResult<app::operations::OperationResult>
    decode_result(const std::span<const std::uint8_t> payload)
    {
        return decode<app::operations::OperationResult>(
            payload,
            [](detail::Reader& reader) -> app::operations::OperationResult
            {
                require_header(reader, PayloadKind::Result);
                switch (reader.read_u8())
                {
                case 0:
                    return app::operations::OperationFailure{reader.read_string()};
                case 1:
                    return app::operations::TranslateResult{detail::read_layout(reader)};
                case 2:
                    return app::operations::RewriteResult{reader.read_string()};
                case 3:
                {
                    app::operations::CompareResult result;
                    switch (reader.read_u8())
                    {
                    case 0:
                        result.comparison = detail::read_comparison(reader);
                        break;
                    case 1:
                        result.comparison = detail::read_omega_comparison(reader);
                        break;
                    default:
                        throw std::runtime_error("Protocol comparison result type is invalid.");
                    }

                    return result;
                }
                default:
                    throw std::runtime_error("Protocol result type is invalid.");
                }
            }
        );
    }
}
