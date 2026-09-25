// Defines rendering-independent UI data for language comparisons.
#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace automata::analysis
{
    struct RegexComparison;
    struct OmegaRegexComparison;
}

namespace ui::comparison
{
    // UI-facing classification of the relation between two languages.
    enum class RelationKind : std::uint8_t
    {
        Equivalent,
        Complement,
        Disjoint,
        LeftSubsetRight,
        RightSubsetLeft,
        Overlap
    };

    // Labeled example word displayed for one set region.
    struct Witness
    {
        std::string label;
        std::string word;
    };

    // UI-facing comparison data without rendering-library dependencies.
    struct ComparisonModel
    {
        RelationKind kind = RelationKind::Overlap;

        std::optional<Witness> only_left;
        std::optional<Witness> only_right;
        std::optional<Witness> intersection;
        std::optional<Witness> neither;

        bool left_empty = false;
        bool right_empty = false;
        bool left_universal = false;
        bool right_universal = false;
        bool infinite_words = false;
    };

    // Adapts the computation result to the smaller UI-facing model.
    ComparisonModel make_comparison_model(const automata::analysis::RegexComparison& comparison);
    // Adapts the omega-word computation result to the smaller UI-facing model.
    ComparisonModel
    make_comparison_model(const automata::analysis::OmegaRegexComparison& comparison);

    // Returns a human-readable description of the relation between both languages.
    [[nodiscard]] std::string describe_relation(const ComparisonModel& model);
}
