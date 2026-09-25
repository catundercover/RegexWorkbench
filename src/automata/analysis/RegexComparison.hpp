// Declares regular-language comparison and witness reporting.
#pragma once

#include "automata/model/Nfa.hpp"
#include "regex/model/Expression.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace automata::analysis
{
    // Classifies the set relationship between two regular languages.
    enum class LanguageRelation : std::uint8_t
    {
        Equivalent,
        Complement,
        Disjoint,
        LeftSubsetRight,
        RightSubsetLeft,
        Overlap
    };

    // Describes a language relation and shortest words in its four set regions.
    struct RegexComparison
    {
        LanguageRelation relation = LanguageRelation::Overlap;

        std::optional<std::string> left_only_witness;
        std::optional<std::string> right_only_witness;
        std::optional<std::string> intersection_witness;
        std::optional<std::string> neither_witness;

        bool left_empty = false;
        bool right_empty = false;
        bool left_universal = false;
        bool right_universal = false;
    };

    // Classifies both languages and supplies shortest witnesses for each region.
    [[nodiscard]] RegexComparison compare(
        const regex::Expression& left,
        const regex::Expression& right,
        const Alphabet& extra_alphabet = {}
    );
}
