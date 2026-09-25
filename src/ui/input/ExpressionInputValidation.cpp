// Implements rendering-independent source-aware expression validation for editors.
#include "ui/input/ExpressionInputValidation.hpp"

#include "regex/parsing/InputNormalization.hpp"
#include "regex/parsing/OmegaRegexParser.hpp"
#include "regex/parsing/ParsingDiagnostics.hpp"
#include "regex/parsing/RegexParser.hpp"

#include <cctype>
#include <sstream>
#include <utility>

namespace ui::input
{
    namespace
    {
        // Makes low-level parser terminology actionable for an expression editor.
        std::string diagnostic_description(
            const std::string_view text,
            const regex::parsing::ParseError& error,
            const app::operations::ExpressionFlavor flavor
        )
        {
            const std::size_t first_line_end = error.message.find('\n');
            std::string description = error.message.substr(0, first_line_end);
            if (error.byte_offset >= text.size())
            {
                return "The expression is incomplete; add the missing operand or closing "
                       "parenthesis";
            }
            if (description == "expected EOF")
            {
                return "Unexpected symbol or operator";
            }
            if (description == "invalid syntax")
            {
                return "The expression contains an unsupported symbol or incomplete operator";
            }
            if (description == "expected an omega regular expression")
            {
                return "Expected an infinite-word expression such as r^ω";
            }
            if (description == "expected the end of the omega regular expression")
            {
                return "Unexpected symbol or operator in the infinite-word expression";
            }
            if (description == "expected ')' after the omega expression")
            {
                return "Missing ')' after the infinite-word expression";
            }

            constexpr std::string_view OmegaPhrase = "omega expression";
            if (flavor == app::operations::ExpressionFlavor::OmegaRegex)
            {
                if (const std::size_t omega = description.find(OmegaPhrase);
                    omega != std::string::npos)
                {
                    description.replace(omega, OmegaPhrase.size(), "infinite-word expression");
                }
            }
            if (!description.empty())
            {
                description.front() = static_cast<char>(
                    std::toupper(static_cast<unsigned char>(description.front()))
                );
            }
            return description;
        }

        // Formats one parser diagnostic with source coordinates and an actionable description.
        std::string diagnostic_summary(
            const std::string_view text,
            const regex::parsing::ParseError& error,
            const app::operations::ExpressionFlavor flavor
        )
        {
            std::ostringstream output;
            output << "Syntax error at "
                   << (error.line == 1 ? "column " + std::to_string(error.column)
                                       : "line " + std::to_string(error.line) + ", column " +
                                             std::to_string(error.column))
                   << ": " << diagnostic_description(text, error, flavor) << '.';
            return output.str();
        }

        // Returns specialized feedback for an omega-power operator used in finite-word mode.
        std::optional<SyntaxFeedback> finite_omega_power_feedback(const std::string_view text)
        {
            const std::size_t omega_power = text.find("^ω");
            if (omega_power == std::string_view::npos)
            {
                return std::nullopt;
            }

            const regex::parsing::ParseError position = regex::parsing::detail::make_parse_error(
                text, omega_power, "omega power is unavailable in finite-word mode"
            );
            std::ostringstream message;
            message << "Syntax error at "
                    << (position.line == 1 ? "column " + std::to_string(position.column)
                                           : "line " + std::to_string(position.line) + ", column " +
                                                 std::to_string(position.column))
                    << ": ^ω is only valid for infinite words; switch to Infinite words or remove "
                       "the operator.";
            return SyntaxFeedback{SyntaxValidity::Invalid, message.str(), omega_power};
        }
    }

    SyntaxFeedback validate_expression_syntax(
        const std::string_view text, const app::operations::ExpressionFlavor flavor
    )
    {
        if (regex::parsing::normalize_input(text).empty())
        {
            return {};
        }

        if (flavor == app::operations::ExpressionFlavor::FiniteRegex)
        {
            if (std::optional<SyntaxFeedback> omega_power = finite_omega_power_feedback(text))
            {
                return std::move(*omega_power);
            }
        }

        if (flavor == app::operations::ExpressionFlavor::OmegaRegex)
        {
            const regex::parsing::omega::ParseResult parsed = regex::parsing::omega::parse(text);
            if (parsed.success())
            {
                return {SyntaxValidity::Valid, {}, std::nullopt};
            }
            return {
                SyntaxValidity::Invalid,
                parsed.error.has_value() ? diagnostic_summary(text, *parsed.error, flavor)
                                         : "The infinite-word expression has invalid syntax.",
                parsed.error.has_value() ? std::optional<std::size_t>{parsed.error->byte_offset}
                                         : std::nullopt
            };
        }

        const regex::parsing::ParseResult parsed = regex::parsing::parse(text);
        if (parsed.success())
        {
            return {SyntaxValidity::Valid, {}, std::nullopt};
        }
        return {
            SyntaxValidity::Invalid,
            parsed.error.has_value() ? diagnostic_summary(text, *parsed.error, flavor)
                                     : "The regular expression has invalid syntax.",
            parsed.error.has_value() ? std::optional<std::size_t>{parsed.error->byte_offset}
                                     : std::nullopt
        };
    }
}
