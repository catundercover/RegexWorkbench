// Declares semantics-preserving normalization for omega expressions.
#pragma once

#include "regex/model/OmegaExpression.hpp"

namespace regex::simplification::omega
{
    // Applies safe local identities and returns a normalized immutable tree.
    [[nodiscard]] regex::omega::Expression normalize(const regex::omega::Expression& expression);
}
