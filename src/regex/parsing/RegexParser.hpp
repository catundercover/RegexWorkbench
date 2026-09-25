// Declares the source-aware regular-expression parser.
#pragma once

#include "regex/model/Expression.hpp"
#include "regex/parsing/ParsingDiagnostics.hpp"

#include <optional>
#include <string_view>

namespace regex::parsing
{
    // Contains either a parsed expression or a diagnostic.
    struct ParseResult
    {
        std::optional<Expression> expression;
        std::optional<ParseError> error;

        // Returns whether exactly a parsed expression is present.
        [[nodiscard]] bool success() const
        {
            return expression.has_value() && !error.has_value();
        }
    };

    // Normalizes and parses a complete regular expression.
    [[nodiscard]] ParseResult parse(std::string_view input);
}
