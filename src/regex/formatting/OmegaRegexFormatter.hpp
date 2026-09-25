// Declares canonical formatting for omega regular expressions.
#pragma once

#include "regex/model/OmegaExpression.hpp"

#include <string>

namespace regex::formatting::omega
{
    // Formats an omega AST as syntax that the omega parser accepts again.
    [[nodiscard]] std::string format(const regex::omega::Expression& expression);
}
