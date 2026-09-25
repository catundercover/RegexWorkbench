// Defines whether an operation works on finite or omega regular expressions.
#pragma once

#include <cstdint>

namespace app::operations
{
    // Selects the language universe and parser/backend used by operations.
    enum class ExpressionFlavor : std::uint8_t
    {
        FiniteRegex,
        OmegaRegex
    };
}