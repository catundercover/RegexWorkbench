#include "regex/parsing/InputNormalization.hpp"
#include "regex/parsing/RegexGrammar.hpp"
#include "RegexParser.hpp"

#include <cstddef>
#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/input/string_input.hpp>
#include <sstream>
#include <string_view>
#include <utility>

namespace regex::parsing
{
    namespace
    {
        // Converts a parser pointer into a byte offset from its input start.
        template <typename Character>
        std::size_t offset_from_begin(const Character* begin, const Character* position)
        {
            return static_cast<std::size_t>(position - begin);
        }

        // Extracts a lexy diagnostic when the error type exposes one.
        template <typename Error>
        std::string describe(const Error& error)
        {
            if constexpr (requires { error.message(); })
            {
                std::ostringstream message;
                message << error.message();
                return message.str();
            }
            return "invalid syntax";
        }

        // Returns a domain-specific finite-regex diagnostic when the failure is recognizable.
        std::string describe_finite_parse_error(
            const std::string_view normalized,
            const std::size_t offset,
            std::string fallback
        )
        {
            if (normalized.empty())
            {
                return "expected a regular expression";
            }

            const char current = offset < normalized.size() ? normalized[offset] : '\0';
            const char previous = offset > 0 && offset - 1 < normalized.size()
                                      ? normalized[offset - 1]
                                      : '\0';

            if ((offset < normalized.size() && normalized.substr(offset).starts_with("^ω")) ||
               (offset > 0 && offset - 1 < normalized.size() &&
                normalized.substr(offset - 1).starts_with("^ω")))
            {
                return "omega is only valid in omega regular-expression mode";
            }

            if (offset < normalized.size() && normalized.substr(offset).starts_with("ω"))
            {
                return "omega is only valid in omega regular-expression mode";
            }

            if (previous == '^')
            {
                return "expected a finite unsigned exponent after '^'";
            }

            if (previous == '|')
            {
                return "expected a regular expression after '|'";
            }

            if (previous == '&')
            {
                return "expected a regular expression after '&'";
            }

            if (current == '|')
            {
                return "expected a regular expression before '|'";
            }

            if (current == '&')
            {
                return "expected a regular expression before '&'";
            }

            if (current == ')')
            {
                return "unexpected ')'";
            }

            if (current == '^')
            {
                return "expected a finite unsigned exponent after '^'";
            }

            if (current != '\0')
            {
                return "unsupported token in regular expression";
            }

            return fallback;
        }
    }

    ParseResult parse(const std::string_view input)
    {
        ParseResult result;
        const detail::NormalizedInput normalized = detail::normalize_with_mapping(input);
        auto parser_input =
            lexy::string_input<lexy::utf8_encoding>(normalized.text.data(), normalized.text.size());

        const auto error_handler = lexy::callback<void>(
            [&](const auto& context, const auto& error)
            {
                (void)context;
                if (result.error.has_value())
                {
                    return;
                }
                const std::size_t normalized_offset =
                    offset_from_begin(parser_input.data(), error.position());
                result.error = detail::make_parse_error(
                    input,
                    normalized.original_offset(normalized_offset),
                    describe_finite_parse_error(
                        normalized.text, normalized_offset, describe(error)
                    )
                );
            }
        );

        auto parsed = lexy::parse<detail::Root>(parser_input, error_handler);
        if (parsed.has_value() && !result.error.has_value())
        {
            result.expression = std::move(parsed).value();
        }
        return result;
    }
}