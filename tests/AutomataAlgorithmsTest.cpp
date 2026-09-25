// Verifies automata translation, conversion, and comparison algorithms.
#include "automata/analysis/RegexComparison.hpp"
#include "automata/conversion/RegexToNfa.hpp"
#include "automata/dot/DotExporter.hpp"
#include "automata/rewrite/RegexRewriter.hpp"
#include "automata/translation/RegexTranslation.hpp"
#include "regex/parsing/RegexParser.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
    // Reports a failed assertion and returns its condition.
    bool check(bool condition, const std::string& message)
    {
        if (!condition)
            std::cerr << message << '\n';
        return condition;
    }

    // Parses a required expression used by an automata test.
    regex::Expression parse(const std::string& text)
    {
        regex::parsing::ParseResult result = regex::parsing::parse(text);
        if (!result.expression.has_value() || result.error.has_value())
            throw std::runtime_error("Test expression did not parse: " + text);
        return result.expression.value();
    }

    // Compares two expressions and checks the expected language relation.
    bool has_relation(
        const std::string& left,
        const std::string& right,
        automata::analysis::LanguageRelation expected,
        const automata::Alphabet& alphabet = {}
    )
    {
        return automata::analysis::compare(parse(left), parse(right), alphabet).relation ==
               expected;
    }
}

// Runs deterministic automata algorithm and witness checks.
int main()
{
    const automata::Nfa sigma = automata::conversion::to_nfa(parse("Σ"), {'a', 'b'});
    if (!check(!sigma.accepts(""), "Sigma accepted the empty word.") ||
        !check(sigma.accepts("a") && sigma.accepts("b"), "Sigma rejected an alphabet symbol.") ||
        !check(!sigma.accepts("aa"), "Sigma accepted a word longer than one symbol."))
    {
        return 1;
    }

    const automata::Nfa repeated = automata::translation::minimal_dfa(parse("(a|b)*abb"));
    if (!check(repeated.accepts("abb"), "The DFA rejected a word in the language.") ||
        !check(repeated.accepts("ababb"), "The DFA rejected a repeated-prefix word.") ||
        !check(!repeated.accepts("aba"), "The DFA accepted a word outside the language."))
    {
        return 1;
    }

    using enum automata::analysis::LanguageRelation;
    if (!check(has_relation("a|a", "a", Equivalent), "Equivalent languages were misclassified.") ||
        !check(
            has_relation("a", "!a", Complement, {'a', 'b'}), "Complements were misclassified."
        ) ||
        !check(has_relation("!!a", "a", Equivalent, {'a', 'b'}), "Double complement was not equivalent to the operand.") ||
        !check(has_relation("!!!a", "!a", Equivalent, {'a', 'b'}), "Triple complement was not equivalent to one complement.") ||
        !check(has_relation("a", "b", Disjoint), "Disjoint languages were misclassified.") ||
        !check(has_relation("a", "a|b", LeftSubsetRight), "Left subset was misclassified.") ||
        !check(has_relation("a|b", "a", RightSubsetLeft), "Right subset was misclassified.") ||
        !check(has_relation("a|b", "b|c", Overlap), "Overlapping languages were misclassified.") ||
        !check(
            has_relation("Σ", "a|b", Equivalent, {'a', 'b'}),
            "Sigma did not equal the set of one-symbol words."
        ) ||
        !check(
            has_relation("Σ", "(a|b)*", LeftSubsetRight, {'a', 'b'}),
            "Sigma was treated as the Kleene closure of the alphabet."
        ))
    {
        return 1;
    }

    const automata::analysis::RegexComparison overlap =
        automata::analysis::compare(parse("a|b"), parse("b|c"));
    if (!check(overlap.left_only_witness.has_value(), "Overlap has no left-only witness.") ||
        !check(overlap.right_only_witness.has_value(), "Overlap has no right-only witness.") ||
        !check(overlap.intersection_witness.has_value(), "Overlap has no shared witness.") ||
        !check(overlap.neither_witness.has_value(), "Overlap has no outside witness."))
    {
        return 1;
    }

    automata::rewrite::Options keep_abbreviations;

    if (!check(
            automata::rewrite::apply(parse("a|∅"), keep_abbreviations) == "a",
            "Unconditional rewrite simplification changed."
        ) ||
        !check(
            automata::rewrite::apply(parse("Σ"), keep_abbreviations, {'a', 'b'}) == "Σ",
            "Sigma was removed when its abbreviation was kept."
        ))
    {
        return 1;
    }

    keep_abbreviations.remove_any_symbol = true;
    const std::string expanded_sigma =
        automata::rewrite::apply(parse("Σ"), keep_abbreviations, {'b', 'a'});
    if (!check(expanded_sigma == "a|b", "Sigma did not expand to a sorted alternation."))
    {
        return 1;
    }

    if (!check(
            automata::rewrite::apply(parse("a+"), keep_abbreviations) == "a+",
            "Plus was removed when its abbreviation was kept."
        ) ||
        !check(
            automata::rewrite::apply(parse("a^2"), keep_abbreviations) == "a^2",
            "Power was removed when its abbreviation was kept."
        ) ||
        !check(
            automata::rewrite::apply(parse("a&b"), keep_abbreviations) == "a&b",
            "Intersection was removed when its abbreviation was kept."
        ) ||
        !check(
            automata::rewrite::apply(parse("!a"), keep_abbreviations, {'a', 'b'}) == "!a",
            "Complement was removed when its abbreviation was kept."
        ))
    {
        return 1;
    }

    automata::rewrite::Options remove_abbreviations = keep_abbreviations;
    remove_abbreviations.remove_plus = true;
    remove_abbreviations.remove_power = true;
    remove_abbreviations.remove_intersection = true;
    remove_abbreviations.remove_complement = true;

    const std::string complement_free =
        automata::rewrite::apply(parse("!a"), remove_abbreviations, {'a', 'b'});
    if (!check(
            automata::rewrite::apply(parse("a+"), remove_abbreviations) == "aa*",
            "Plus unabbreviation changed."
        ) ||
        !check(
            automata::rewrite::apply(parse("a^2"), remove_abbreviations) == "aa",
            "Power unabbreviation changed."
        ) ||
        !check(
            automata::rewrite::apply(parse("a&b"), remove_abbreviations) == "∅",
            "Intersection unabbreviation changed."
        ) ||
        !check(
            complement_free.find('!') == std::string::npos,
            "Complement unabbreviation still contains a complement operator."
        ))
    {
        return 1;
    }

    bool power_budget_rejected = false;
    try
    {
        (void)automata::rewrite::apply(parse("a^4096"), remove_abbreviations);
    }
    catch (const std::runtime_error& error)
    {
        power_budget_rejected =
            std::string(error.what()).find("4096 expression nodes") != std::string::npos;
    }
    if (!check(power_budget_rejected, "Oversized power expansion had no clear error."))
    {
        return 1;
    }

    automata::Nfa dot_nfa;
    const automata::StateId start = dot_nfa.add_state();
    const automata::StateId final = dot_nfa.add_state();
    dot_nfa.start = start;
    dot_nfa.finals.insert(final);
    dot_nfa.add_transition(start, 'b', final);
    dot_nfa.add_transition(start, 'a', final);
    const std::string first_dot = automata::dot::to_dot(dot_nfa);
    const std::string second_dot = automata::dot::to_dot(dot_nfa);
    if (!check(first_dot == second_dot, "DOT output was not deterministic.") ||
        !check(first_dot.find("label=\"a,b\"") != std::string::npos, "DOT labels were not sorted."))
    {
        return 1;
    }

    return 0;
}
