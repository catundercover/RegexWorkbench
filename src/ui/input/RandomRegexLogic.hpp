// Declares rendering-independent adaptation of generated expressions to the active word domain.
#pragma once

#include "app/operations/ExpressionFlavor.hpp"

#include <string>

namespace ui::input
{
    // Keeps finite regexes unchanged and turns them into valid omega repetitions in omega mode.
    [[nodiscard]] std::string
    adapt_generated_expression(std::string expression, app::operations::ExpressionFlavor flavor);
}
