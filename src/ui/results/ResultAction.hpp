/// @file
/// Defines actions emitted while rendering operation results.
#pragma once

#include <cstdint>
namespace ui::results
{
    /// Identifies an action requested by a result widget.
    enum class ResultActionKind : std::uint8_t
    {
        None,
        CancelOperation
    };

    /// Carries an action requested by an operation result.
    struct ResultAction
    {
        ResultActionKind kind = ResultActionKind::None;
    };
}
