// Implements generated-expression adaptation for finite and infinite words.
#include "ui/input/RandomRegexLogic.hpp"

namespace ui::input
{
    std::string adapt_generated_expression(
        std::string expression, const app::operations::ExpressionFlavor flavor
    )
    {
        if (flavor == app::operations::ExpressionFlavor::OmegaRegex)
        {
            return "(" + expression + ")^ω";
        }
        return expression;
    }
}
