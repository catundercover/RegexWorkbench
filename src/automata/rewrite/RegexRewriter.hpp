// Declares configurable regular-expression operator expansion.
#pragma once

#include "automata/model/Nfa.hpp"
#include "regex/model/Expression.hpp"

#include <cstddef>
#include <string>

namespace automata::rewrite
{
    // Maximum expression-tree size produced by a rewrite.
    inline constexpr std::size_t MaxOutputNodes = 4096;

    // Selects which optional operators should be expanded.
    struct Options
    {
        bool remove_complement = false;
        bool remove_intersection = false;
        bool remove_power = false;
        bool remove_plus = false;
        bool remove_any_symbol = false;
    };

    // Normalizes once before and after applying the requested unabbreviations.
    [[nodiscard]] std::string apply(
        const regex::Expression& expression,
        const Options& options,
        const Alphabet& extra_alphabet = {}
    );
}
