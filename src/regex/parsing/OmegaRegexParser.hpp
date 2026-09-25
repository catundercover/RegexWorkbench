// Declares the source-aware omega-regular-expression parser.
#pragma once

#include "regex/model/OmegaExpression.hpp"
#include "regex/parsing/ParsingDiagnostics.hpp"

#include <optional>
#include <string_view>

namespace regex::parsing::omega
{
    // Contains either a parsed omega AST or a source-positioned diagnostic.
    struct ParseResult
    {
        std::optional<regex::omega::Expression> expression;
        std::optional<ParseError> error;

        // Returns whether parsing produced an expression without a diagnostic.
        [[nodiscard]] bool success() const
        {
            return expression.has_value() && !error.has_value();
        }
    };

    // Normalizes and parses one complete omega regular expression.
    [[nodiscard]] ParseResult parse(std::string_view input);
}
