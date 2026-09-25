// Property-tests language preservation across automata algorithms.
#include "automata/analysis/RegexComparison.hpp"
#include "automata/conversion/RegexToNfa.hpp"
#include "automata/rewrite/RegexRewriter.hpp"
#include "automata/translation/RegexTranslation.hpp"
#include "regex/generation/RandomRegexGenerator.hpp"
#include "support/TestSupport.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
    // Returns the shared binary alphabet used by property checks.
    const automata::Alphabet& alphabet()
    {
        static const automata::Alphabet value{'a', 'b'};
        return value;
    }

    // Returns all binary words through length four.
    const std::vector<std::string>& sample_words()
    {
        static const std::vector<std::string> value = test_support::words_through_length("ab", 4);
        return value;
    }

    // Checks language agreement among direct, epsilon-free, and minimal automata.
    bool automata_agree(const regex::Expression& expression, const std::string_view label)
    {
        const automata::Nfa direct = automata::conversion::to_nfa(expression, alphabet());
        const automata::Nfa epsilon_free =
            automata::translation::epsilon_free_nfa(expression, alphabet());
        const automata::Nfa minimal = automata::translation::minimal_dfa(expression, alphabet());

        for (const std::string& word : sample_words())
        {
            const bool expected = direct.accepts(word);
            if (!test_support::check(
                    epsilon_free.accepts(word) == expected,
                    std::string(label) + ": epsilon-removal changed acceptance of '" + word + "'."
                ) ||
                !test_support::check(
                    minimal.accepts(word) == expected,
                    std::string(label) + ": determinization changed acceptance of '" + word + "'."
                ))
            {
                return false;
            }
        }
        return true;
    }

    // Maps an asymmetric relation to the relation expected after swapping operands.
    automata::analysis::LanguageRelation
    swapped_relation(const automata::analysis::LanguageRelation relation)
    {
        using enum automata::analysis::LanguageRelation;
        switch (relation)
        {
        case LeftSubsetRight:
            return RightSubsetLeft;
        case RightSubsetLeft:
            return LeftSubsetRight;
        default:
            return relation;
        }
    }

    // Checks that an available witness belongs to its reported set region.
    bool witness_matches_region(
        const std::optional<std::string>& witness,
        const automata::Nfa& left,
        const automata::Nfa& right,
        const bool expected_left,
        const bool expected_right,
        const std::string_view region
    )
    {
        if (!witness)
        {
            return true;
        }
        return test_support::check(
            left.accepts(*witness) == expected_left && right.accepts(*witness) == expected_right,
            std::string(region) + " witness does not belong to its reported language region."
        );
    }

    // Checks relation symmetry and the membership of every reported witness.
    bool
    comparison_is_consistent(const std::string_view left_text, const std::string_view right_text)
    {
        const regex::Expression left = test_support::parse_expression(left_text);
        const regex::Expression right = test_support::parse_expression(right_text);
        const automata::analysis::RegexComparison comparison =
            automata::analysis::compare(left, right, alphabet());
        const automata::analysis::RegexComparison swapped =
            automata::analysis::compare(right, left, alphabet());

        const automata::Nfa left_nfa = automata::conversion::to_nfa(left, alphabet());
        const automata::Nfa right_nfa = automata::conversion::to_nfa(right, alphabet());

        return test_support::check(
                   swapped.relation == swapped_relation(comparison.relation),
                   "Swapping comparison operands produced an inconsistent relation."
               ) &&
               witness_matches_region(
                   comparison.left_only_witness, left_nfa, right_nfa, true, false, "Left-only"
               ) &&
               witness_matches_region(
                   comparison.right_only_witness, left_nfa, right_nfa, false, true, "Right-only"
               ) &&
               witness_matches_region(
                   comparison.intersection_witness, left_nfa, right_nfa, true, true, "Intersection"
               ) &&
               witness_matches_region(
                   comparison.neither_witness, left_nfa, right_nfa, false, false, "Neither"
               ) &&
               test_support::check(
                   swapped.left_only_witness == comparison.right_only_witness &&
                       swapped.right_only_witness == comparison.left_only_witness,
                   "Swapping comparison operands did not swap the exclusive witnesses."
               );
    }

    // Checks that expanding every optional operator preserves sampled acceptance.
    bool rewrite_preserves_language(const std::string_view text)
    {
        const regex::Expression original = test_support::parse_expression(text);
        automata::rewrite::Options options;
        options.remove_complement = true;
        options.remove_intersection = true;
        options.remove_power = true;
        options.remove_plus = true;
        options.remove_any_symbol = true;

        const std::string rewritten_text = automata::rewrite::apply(original, options, alphabet());
        const regex::Expression rewritten = test_support::parse_expression(rewritten_text);
        const automata::Nfa original_nfa = automata::conversion::to_nfa(original, alphabet());
        const automata::Nfa rewritten_nfa = automata::conversion::to_nfa(rewritten, alphabet());
        for (const std::string& word : sample_words())
        {
            if (!test_support::check(
                    original_nfa.accepts(word) == rewritten_nfa.accepts(word),
                    std::string(text) + ": rewrite changed acceptance of '" + word + "'."
                ))
            {
                return false;
            }
        }
        return true;
    }
}

// Runs deterministic and generated cross-algorithm property checks.
int main()
{
    constexpr std::array<std::string_view, 13> Expressions{
        "∅",
        "ε",
        "Σ",
        "a|b",
        "ab*",
        "(a|b)*abb",
        "a+",
        "(ab)^2",
        "!a",
        "a&b",
        "!(a|b)",
        "Σ^2",
        "(a|ε)&!b"
    };
    for (const std::string_view text : Expressions)
    {
        if (!automata_agree(test_support::parse_expression(text), text))
        {
            return 1;
        }
    }

    regex::generation::Config generator_config;
    generator_config.max_depth = 4;
    for (std::uint32_t seed = 0; seed < 24; ++seed)
    {
        regex::generation::Generator generator(seed);
        if (!automata_agree(generator.generate(generator_config), "generated expression"))
        {
            return 1;
        }
    }

    constexpr std::array<std::pair<std::string_view, std::string_view>, 6> Comparisons{
        std::pair{"a|a", "a"},
        std::pair{"a", "a|b"},
        std::pair{"a|b", "a"},
        std::pair{"a", "b"},
        std::pair{"a|b", "b|ab"},
        std::pair{"a", "!a"}
    };
    for (const auto& [left, right] : Comparisons)
    {
        if (!comparison_is_consistent(left, right))
        {
            return 1;
        }
    }

    constexpr std::array<std::string_view, 7> Rewrites{
        "a+", "(a|b)^2", "a&b", "!a", "Σ", "(a|ε)&!b", "(!(a|b))*"
    };
    for (const std::string_view text : Rewrites)
    {
        if (!rewrite_preserves_language(text))
        {
            return 1;
        }
    }

    return 0;
}
