// Declares language-preserving regular-expression normalization.
#pragma once

#include "regex/model/Expression.hpp"

namespace regex::simplification
{
    // Applies language-preserving identities without expanding optional operators.
    [[nodiscard]] Expression normalize(const Expression& expression);
}
