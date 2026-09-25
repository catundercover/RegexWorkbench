// Declares omega-regular-language comparison and ultimately periodic witnesses.
#pragma once

#include "automata/analysis/RegexComparison.hpp"
#include "automata/model/Nfa.hpp"
#include "regex/model/OmegaExpression.hpp"

#include <optional>
#include <string>

namespace automata::analysis
{
    // Ultimately periodic omega-word witness of the form prefix(cycle)^omega.
    struct OmegaWitness
    {
        std::string prefix;
        std::string cycle;
    };

    // Describes a relation between two omega-regular languages and witnesses
    // for the four Venn regions.
    struct OmegaRegexComparison
    {
        LanguageRelation relation = LanguageRelation::Overlap;

        std::optional<OmegaWitness> left_only_witness;
        std::optional<OmegaWitness> right_only_witness;
        std::optional<OmegaWitness> intersection_witness;
        std::optional<OmegaWitness> neither_witness;

        bool left_empty = false;
        bool right_empty = false;
        bool left_universal = false;
        bool right_universal = false;
    };

    // Classifies both omega languages and supplies ultimately periodic
    // witnesses for each non-empty region.
    [[nodiscard]] OmegaRegexComparison compare(
        const regex::omega::Expression& left,
        const regex::omega::Expression& right,
        const Alphabet& extra_alphabet = {}
    );
}