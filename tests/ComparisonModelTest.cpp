// Verifies adaptation and descriptions of comparison UI models.
#include "ui/comparison/ComparisonModel.hpp"

#include "automata/analysis/OmegaRegexComparison.hpp"
#include "automata/analysis/RegexComparison.hpp"

#include <iostream>
#include <string>

namespace
{
    // Reports a failed assertion and returns its condition.
    bool check(bool condition, const std::string& message)
    {
        if (!condition)
        {
            std::cerr << message << '\n';
        }
        return condition;
    }
}

// Runs comparison-model adaptation and description checks.
int main()
{
    automata::analysis::RegexComparison comparison;
    comparison.relation = automata::analysis::LanguageRelation::Equivalent;
    comparison.left_only_witness = "a";
    comparison.right_only_witness = "b";
    comparison.intersection_witness = "";
    comparison.neither_witness = "c";

    const ui::comparison::ComparisonModel model = ui::comparison::make_comparison_model(comparison);

    if (!check(
            model.kind == ui::comparison::RelationKind::Equivalent,
            "Comparison relation was mapped incorrectly."
        ) ||
        !check(
            model.intersection.has_value() && model.intersection->word == "ε",
            "The empty-word witness was not formatted as epsilon."
        ) ||
        !check(
            model.only_left.has_value() && model.only_left->label == "w1",
            "The left witness label is incorrect."
        ) ||
        !check(
            model.only_right.has_value() && model.only_right->label == "w3",
            "The right witness label is incorrect."
        ) ||
        !check(
            model.intersection.has_value() && model.intersection->label == "w2",
            "The shared label is incorrect."
        ) ||
        !check(
            model.neither.has_value() && model.neither->label == "w4",
            "The neither label is incorrect."
        ) ||
        !check(
            ui::comparison::describe_relation(model) ==
                "The two regular expressions describe the same language.",
            "The equivalent-relation description is incorrect."
        ))
    {
        return 1;
    }

    ui::comparison::ComparisonModel universal_and_empty;
    universal_and_empty.left_universal = true;
    universal_and_empty.right_empty = true;
    if (!check(
            ui::comparison::describe_relation(universal_and_empty) ==
                "The first language contains all words, while the second language is empty. "
                "Therefore, the two languages are complements of one another.",
            "Universal/empty relation description is incorrect."
        ))
    {
        return 1;
    }

    ui::comparison::ComparisonModel subset;
    subset.kind = ui::comparison::RelationKind::LeftSubsetRight;
    if (!check(
            ui::comparison::describe_relation(subset) ==
                "The language of the first regex is a proper subset of the language of the second "
                "regex.",
            "Subset relation description is incorrect."
        ))
    {
        return 1;
    }

    automata::analysis::OmegaRegexComparison omega_comparison;
    omega_comparison.relation = automata::analysis::LanguageRelation::Overlap;
    omega_comparison.left_only_witness = automata::analysis::OmegaWitness{"a", "b"};
    omega_comparison.intersection_witness = automata::analysis::OmegaWitness{"", "ab"};

    const ui::comparison::ComparisonModel omega_model =
        ui::comparison::make_comparison_model(omega_comparison);

    if (!check(
            omega_model.only_left.has_value() && omega_model.only_left->word == "a(b)^ω",
            "Omega left-only witness was not formatted as ultimately periodic."
        ) ||
        !check(
            omega_model.intersection.has_value() && omega_model.intersection->word == "(ab)^ω",
            "Omega intersection witness was not formatted as ultimately periodic."
        ) ||
        !check(
            omega_model.infinite_words,
            "The omega comparison model was not marked as an infinite-word comparison."
        ) ||
        !check(
            ui::comparison::describe_relation(omega_model) ==
                "The two omega regular expressions are incomparable.",
            "The omega relation description used finite-word terminology."
        ))
    {
        return 1;
    }

    return 0;
}
