// Declares rendering-independent source-aware expression validation for editors.
#pragma once

#include "app/operations/ExpressionFlavor.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace ui::input
{
    // Classifies lightweight parser feedback for an expression editor.
    enum class SyntaxValidity : std::uint8_t
    {
        Empty,
        Valid,
        Invalid
    };

    // Compact parser feedback suitable for presentation below an editor.
    struct SyntaxFeedback
    {
        SyntaxValidity validity = SyntaxValidity::Empty;
        std::string message;
        std::optional<std::size_t> error_byte_offset;
    };

    // Validates one complete expression in the selected word domain.
    [[nodiscard]] SyntaxFeedback
    validate_expression_syntax(std::string_view text, app::operations::ExpressionFlavor flavor);
}
