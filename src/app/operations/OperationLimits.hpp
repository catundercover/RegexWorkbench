// Defines resource limits shared by operation validation and execution.
#pragma once

#include <chrono>
#include <cstddef>

namespace app::operations::limits
{
    // Maximum wall-clock time for parsing through final result construction.
    inline constexpr std::chrono::seconds ExecutionTimeout{5};

    // Maximum accepted byte length of one regular-expression input.
    inline constexpr std::size_t MaxRegexInputBytes = 4096;
    // Maximum number of automaton states allowed in a rendered graph.
    inline constexpr std::size_t MaxRenderedStates = 250;
    // Maximum number of automaton transitions allowed in a rendered graph.
    inline constexpr std::size_t MaxRenderedTransitions = 900;
    // Maximum DOT document size accepted by the layout layer.
    inline constexpr std::size_t MaxDotBytes = std::size_t{2} * 1024 * 1024;
}
