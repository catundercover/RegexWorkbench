// Declares in-process Graphviz layout operations.
#pragma once

#include "graph/model/Layout.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

namespace graph::graphviz
{
    // Identifies the stage at which graph layout failed.
    enum class LayoutErrorCode : std::uint8_t
    {
        EmptyInput,
        ContextInitializationFailed,
        InvalidDot,
        LayoutFailed,
        RenderFailed,
        InvalidOutput
    };

    // Contains a stable error category and user-facing diagnostic.
    struct LayoutError
    {
        LayoutErrorCode code;
        std::string message;
    };

    // Result of a layout operation without exceptions crossing API boundaries.
    using LayoutResult = std::variant<Layout, LayoutError>;

    // Computes an automaton layout entirely in-process using Graphviz.
    [[nodiscard]] LayoutResult compute_layout(std::string_view dot);

}
