// Verifies rejection of malformed and oversized protocol payloads.
#include "support/TestSupport.hpp"
#include "worker/protocol/OperationProtocol.hpp"
#include "worker/protocol/PayloadFrame.hpp"
#include "worker/protocol/ProtocolLimits.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <variant>

namespace
{
    // Serialized payload alias used by malformed-message fixtures.
    using worker::protocol::Payload;

    // Returns whether request decoding reports a protocol error.
    bool request_is_rejected(const Payload& payload)
    {
        return std::holds_alternative<worker::protocol::ProtocolError>(
            worker::protocol::decode_request(payload)
        );
    }

    // Returns whether result decoding reports a protocol error.
    bool result_is_rejected(const Payload& payload)
    {
        return std::holds_alternative<worker::protocol::ProtocolError>(
            worker::protocol::decode_result(payload)
        );
    }

    // Checks that every strict prefix of a valid payload is rejected.
    bool all_truncations_are_rejected(const Payload& payload, const bool request)
    {
        for (std::size_t size = 0; size < payload.size(); ++size)
        {
            const std::span prefix(payload.data(), size);
            const bool rejected = request ? std::holds_alternative<worker::protocol::ProtocolError>(
                                                worker::protocol::decode_request(prefix)
                                            )
                                          : std::holds_alternative<worker::protocol::ProtocolError>(
                                                worker::protocol::decode_result(prefix)
                                            );
            if (!rejected)
            {
                return false;
            }
        }
        return true;
    }
}

// Runs malformed, truncated, oversized, and non-finite protocol checks.
int main()
{
    using namespace app::operations;
    using namespace worker::protocol;

    const Payload translate = encode_request(
        TranslateRequest{"a", ExpressionFlavor::FiniteRegex, TranslateMode::Dfa, "b"}
    );
    if (!test_support::check(
            all_truncations_are_rejected(translate, true),
            "A truncated request payload was accepted."
        ))
    {
        return 1;
    }

    Payload malformed = translate;
    malformed[0] ^= 0xFFU;
    if (!test_support::check(
            request_is_rejected(malformed), "Invalid protocol magic was accepted."
        ))
    {
        return 1;
    }
    malformed = translate;
    malformed[4] = 0xFFU;
    if (!test_support::check(
            request_is_rejected(malformed), "An unsupported protocol version was accepted."
        ))
    {
        return 1;
    }
    malformed = translate;
    malformed[5] = 1;
    if (!test_support::check(
            request_is_rejected(malformed), "A result payload kind was accepted as a request."
        ))
    {
        return 1;
    }
    malformed = translate;
    malformed[6] = 0xFFU;
    if (!test_support::check(
            request_is_rejected(malformed), "An unknown request tag was accepted."
        ))
    {
        return 1;
    }
    malformed = translate;
    malformed[12] = 0xFFU;
    if (!test_support::check(
            request_is_rejected(malformed), "An unknown expression flavor was accepted."
        ))
    {
        return 1;
    }
    malformed = translate;
    malformed[13] = 0xFFU;
    if (!test_support::check(
            request_is_rejected(malformed), "An unknown translation mode was accepted."
        ))
    {
        return 1;
    }

    RewriteRequest rewrite;
    rewrite.regex = "a";
    rewrite.flavor = ExpressionFlavor::OmegaRegex;
    Payload encoded_rewrite = encode_request(rewrite);

    Payload invalid_rewrite_flavor = encoded_rewrite;
    invalid_rewrite_flavor[12] = 0xFFU;
    if (!test_support::check(
            request_is_rejected(invalid_rewrite_flavor),
            "An unknown rewrite expression flavor was accepted."
        ))
    {
        return 1;
    }

    encoded_rewrite[encoded_rewrite.size() - 1] = 2;
    if (!test_support::check(
            request_is_rejected(encoded_rewrite), "A non-binary rewrite flag was accepted."
        ))
    {
        return 1;
    }

    Payload invalid_length = translate;
    invalid_length[7] = 0xFFU;
    invalid_length[8] = 0xFFU;
    invalid_length[9] = 0xFFU;
    invalid_length[10] = 0x7FU;
    if (!test_support::check(
            request_is_rejected(invalid_length), "A string length beyond the payload was accepted."
        ))
    {
        return 1;
    }

    const Payload failure = encode_result(OperationFailure{"failure"});
    if (!test_support::check(
            all_truncations_are_rejected(failure, false), "A truncated result payload was accepted."
        ))
    {
        return 1;
    }

    Payload invalid_result_tag = failure;
    invalid_result_tag[6] = 0xFFU;
    if (!test_support::check(
            result_is_rejected(invalid_result_tag), "An unknown result tag was accepted."
        ))
    {
        return 1;
    }

    automata::analysis::RegexComparison comparison;
    const Payload comparison_payload = encode_result(CompareResult{comparison});

    Payload invalid_comparison_kind = comparison_payload;
    invalid_comparison_kind[7] = 0xFFU;
    if (!test_support::check(
            result_is_rejected(invalid_comparison_kind),
            "An unknown comparison result type was accepted."
        ))
    {
        return 1;
    }

    Payload invalid_relation = comparison_payload;
    invalid_relation[8] = 0xFFU;
    if (!test_support::check(
            result_is_rejected(invalid_relation), "An unknown language relation was accepted."
        ))
    {
        return 1;
    }

    Payload invalid_optional = comparison_payload;
    invalid_optional[9] = 2;
    if (!test_support::check(
            result_is_rejected(invalid_optional), "A non-binary optional marker was accepted."
        ))
    {
        return 1;
    }

    automata::analysis::OmegaRegexComparison invalid_omega_comparison;
    invalid_omega_comparison.intersection_witness = automata::analysis::OmegaWitness{"a", ""};
    const Payload empty_cycle = encode_result(CompareResult{invalid_omega_comparison});
    if (!test_support::check(
            result_is_rejected(empty_cycle), "An omega witness with an empty cycle was accepted."
        ))
    {
        return 1;
    }

    graph::Layout non_finite_layout;
    non_finite_layout.bounds.minimum.x = std::numeric_limits<float>::quiet_NaN();
    const Payload non_finite = encode_result(TranslateResult{non_finite_layout});
    if (!test_support::check(
            result_is_rejected(non_finite), "A non-finite graph coordinate was accepted."
        ))
    {
        return 1;
    }

    const Payload oversized(worker::protocol::limits::MaxPayloadBytes + 1, 0);
    if (!test_support::check(
            request_is_rejected(oversized), "A payload above the protocol limit was accepted."
        ))
    {
        return 1;
    }

    constexpr std::uint32_t FramedSize = 0x78563412U;
    const PayloadFrameHeader frame = encode_payload_size(FramedSize);
    if (!test_support::check(
            frame == PayloadFrameHeader{0x12U, 0x34U, 0x56U, 0x78U},
            "The payload frame was not encoded in little-endian order."
        ) ||
        !test_support::check(
            decode_payload_size(frame) == FramedSize, "The payload frame did not round-trip."
        ))
    {
        return 1;
    }

    if constexpr (
        std::numeric_limits<std::size_t>::max() > std::numeric_limits<std::uint32_t>::max()
    )
    {
        bool overflow_rejected = false;
        try
        {
            const std::size_t overflow =
                static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()) + 1;
            (void)encode_payload_size(overflow);
        }
        catch (const std::length_error&)
        {
            overflow_rejected = true;
        }
        if (!test_support::check(
                overflow_rejected, "A payload size wider than the frame was accepted."
            ))
        {
            return 1;
        }
    }

    return 0;
}
