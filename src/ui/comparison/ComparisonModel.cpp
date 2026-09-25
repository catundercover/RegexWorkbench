// Implements adaptation and descriptions for comparison UI data.
#include "ui/comparison/ComparisonModel.hpp"

#include "automata/analysis/OmegaRegexComparison.hpp"
#include "automata/analysis/RegexComparison.hpp"

#include <utility>

namespace ui::comparison
{
    namespace
    {
        // Maps the computation-layer relation enum to its UI counterpart.
        RelationKind relation_kind(automata::analysis::LanguageRelation relation)
        {
            switch (relation)
            {
            case automata::analysis::LanguageRelation::Equivalent:
                return RelationKind::Equivalent;
            case automata::analysis::LanguageRelation::Complement:
                return RelationKind::Complement;
            case automata::analysis::LanguageRelation::Disjoint:
                return RelationKind::Disjoint;
            case automata::analysis::LanguageRelation::LeftSubsetRight:
                return RelationKind::LeftSubsetRight;
            case automata::analysis::LanguageRelation::RightSubsetLeft:
                return RelationKind::RightSubsetLeft;
            case automata::analysis::LanguageRelation::Overlap:
                return RelationKind::Overlap;
            }

            return RelationKind::Overlap;
        }

        // Converts an optional finite word to a labeled witness, displaying empty as epsilon.
        std::optional<Witness>
        make_witness(const std::optional<std::string>& word, std::string label)
        {
            if (!word.has_value())
            {
                return std::nullopt;
            }

            return Witness{std::move(label), word->empty() ? "ε" : *word};
        }

        std::string format_omega_witness(const automata::analysis::OmegaWitness& witness)
        {
            std::string result;

            if (!witness.prefix.empty())
            {
                result += witness.prefix;
            }

            result += "(";
            result += witness.cycle.empty() ? "Σ" : witness.cycle;
            result += ")^ω";

            return result;
        }

        std::optional<Witness> make_witness(
            const std::optional<automata::analysis::OmegaWitness>& witness, std::string label
        )
        {
            if (!witness.has_value())
            {
                return std::nullopt;
            }

            return Witness{std::move(label), format_omega_witness(*witness)};
        }
    }

    ComparisonModel make_comparison_model(const automata::analysis::RegexComparison& comparison)
    {
        ComparisonModel model;
        model.kind = relation_kind(comparison.relation);
        model.only_left = make_witness(comparison.left_only_witness, "w1");
        model.intersection = make_witness(comparison.intersection_witness, "w2");
        model.only_right = make_witness(comparison.right_only_witness, "w3");
        model.neither = make_witness(comparison.neither_witness, "w4");
        model.left_empty = comparison.left_empty;
        model.right_empty = comparison.right_empty;
        model.left_universal = comparison.left_universal;
        model.right_universal = comparison.right_universal;
        return model;
    }

    ComparisonModel
    make_comparison_model(const automata::analysis::OmegaRegexComparison& comparison)
    {
        ComparisonModel model;
        model.kind = relation_kind(comparison.relation);
        model.only_left = make_witness(comparison.left_only_witness, "w1");
        model.intersection = make_witness(comparison.intersection_witness, "w2");
        model.only_right = make_witness(comparison.right_only_witness, "w3");
        model.neither = make_witness(comparison.neither_witness, "w4");
        model.left_empty = comparison.left_empty;
        model.right_empty = comparison.right_empty;
        model.left_universal = comparison.left_universal;
        model.right_universal = comparison.right_universal;
        model.infinite_words = true;
        return model;
    }

    std::string describe_relation(const ComparisonModel& model)
    {
        if (model.left_universal && model.right_universal)
        {
            return model.infinite_words
                       ? "Both omega regular expressions describe all infinite words over the "
                         "alphabet, so the languages are equivalent."
                       : "Both regular expressions describe all words over the alphabet, so the "
                         "languages are equivalent.";
        }

        if (model.left_empty && model.right_empty)
        {
            return model.infinite_words
                       ? "Both omega regular expressions describe the empty language, so the "
                         "languages are equivalent."
                       : "Both regular expressions describe the empty language, so the languages "
                         "are equivalent.";
        }

        if (model.left_universal && model.right_empty)
        {
            return model.infinite_words
                       ? "The first language contains all infinite words, while the second "
                         "language "
                         "is empty. Therefore, the two languages are complements of one another."
                       : "The first language contains all words, while the second language is "
                         "empty. "
                         "Therefore, the two languages are complements of one another.";
        }

        if (model.right_universal && model.left_empty)
        {
            return model.infinite_words
                       ? "The second language contains all infinite words, while the first "
                         "language "
                         "is empty. Therefore, the two languages are complements of one another."
                       : "The second language contains all words, while the first language is "
                         "empty. "
                         "Therefore, the two languages are complements of one another.";
        }

        if (model.left_universal)
        {
            return model.infinite_words
                       ? "The first language contains all infinite words. The second language is a "
                         "subset of it."
                       : "The first language contains all words. The second language is a subset "
                         "of "
                         "it.";
        }

        if (model.right_universal)
        {
            return model.infinite_words
                       ? "The second language contains all infinite words. The first language is a "
                         "subset of it."
                       : "The second language contains all words. The first language is a subset "
                         "of "
                         "it.";
        }

        if (model.left_empty)
        {
            return "The first language is empty and is therefore a subset of the second language.";
        }

        if (model.right_empty)
        {
            return "The second language is empty and is therefore a subset of the first language.";
        }

        switch (model.kind)
        {
        case RelationKind::Equivalent:
            return model.infinite_words
                       ? "The two omega regular expressions describe the same language."
                       : "The two regular expressions describe the same language.";
        case RelationKind::Complement:
            return model.infinite_words
                       ? "The two omega regular expressions are complements of one another."
                       : "The two regular expressions are complements of one another.";
        case RelationKind::Disjoint:
            return "The two languages are disjoint.";
        case RelationKind::LeftSubsetRight:
            return model.infinite_words
                       ? "The language of the first omega regex is a proper subset of the language "
                         "of the second omega regex."
                       : "The language of the first regex is a proper subset of the language of "
                         "the "
                         "second regex.";
        case RelationKind::RightSubsetLeft:
            return model.infinite_words ? "The language of the second omega regex is a proper "
                                          "subset of the language "
                                          "of the first omega regex."
                                        : "The language of the second regex is a proper subset of "
                                          "the language of the "
                                          "first regex.";
        case RelationKind::Overlap:
            return model.infinite_words ? "The two omega regular expressions are incomparable."
                                        : "The two regular expressions are incomparable.";
        }

        return {};
    }
}
