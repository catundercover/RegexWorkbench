// Declares canonical formatting for regular-expression trees.
#pragma once

#include "regex/model/Expression.hpp"

#include <string>

namespace regex::formatting
{
    // Produces a compact expression that can be parsed with the same precedence.
    [[nodiscard]] std::string format(const Expression& expression);
}
