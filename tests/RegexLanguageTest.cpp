// Verifies parser and automaton semantics for the supported regex language.
#include "regex/formatting/RegexFormatter.hpp"
#include "regex/generation/RandomRegexGenerator.hpp"
#include "regex/inspection/ExpressionInspection.hpp"
#include "regex/model/Expression.hpp"
#include "regex/parsing/InputNormalization.hpp"
#include "regex/parsing/RegexParser.hpp"
#include "regex/simplification/RegexSimplifier.hpp"

#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <variant>

namespace
{
    // Reports a failed assertion and returns its condition.
    bool check(const bool condition, const std::string& message)
    {
        if (!condition)
        {
            std::cerr << message << '\n';
        }
        return condition;
    }

    // Parses a required expression used by a language test.
    regex::Expression parse(const std::string& input)
    {
        regex::parsing::ParseResult result = regex::parsing::parse(input);
        if (!result.expression.has_value() || result.error.has_value())
        {
            throw std::runtime_error("Test expression did not parse: " + input);
        }
        return result.expression.value();
    }

    // Checks that parsing fails with a diagnostic containing expected text.
    bool parse_error_contains(const std::string& input, const std::string& expected)
    {
        const regex::parsing::ParseResult result = regex::parsing::parse(input);
        return !result.success() && result.error.has_value() &&
               result.error->message.find(expected) != std::string::npos;
    }

    // Checks the canonical formatting of one parsed expression.
    bool formats_as(const std::string& input, const std::string& expected)
    {
        return regex::formatting::format(parse(input)) == expected;
    }
}

// Runs parser, formatter, and language-semantics checks.
int main()
{
    const regex::Expression nested = regex::make_alternation(
        {regex::make_terminal('a'),
         regex::make_alternation({regex::make_terminal('b'), regex::make_terminal('c')})}
    );
    const std::string nested_text = regex::formatting::format(nested);
    if (!check(nested_text == "a|(b|c)", "Formatting lost explicit AST nesting.") ||
        !check(
            regex::inspection::structurally_equal(nested, parse(nested_text)),
            "Formatting and parsing did not round-trip structurally."
        ) ||
        !check(formats_as("a|b&c", "a|b&c"), "Alternation/intersection precedence changed.") ||
        !check(formats_as("a&bc", "a&bc"), "Intersection/concatenation precedence changed.") ||
        !check(formats_as("!a*", "!a*"), "Complement/repetition precedence changed.") ||
        !check(formats_as("(!a)*", "(!a)*"), "Parenthesized complement formatting changed.") ||
        !check(formats_as("!!a", "a"), "Double complement was not simplified while parsing.") ||
        !check(formats_as("!!!a", "!a"), "Odd complement chain was not simplified to one complement.") ||
        !check(formats_as("~~~~a", "a"), "Even tilde-complement chain was not simplified.") ||
        !check(formats_as("!~!a", "!a"), "Mixed complement spellings were not simplified by parity.") ||
        !check(formats_as("!!(a|b)", "a|b"), "Grouped double complement was not simplified.") ||
        !check(formats_as("!!!(a|b)", "!(a|b)"), "Grouped odd complement chain formatted incorrectly.") ||
        !check(formats_as("a^0", "a^0"), "The parser no longer preserves power syntax.") ||
        !check(formats_as("a²", "a^2"), "A superscript exponent was not normalized.") ||
        !check(formats_as("(ab)¹²", "(ab)^12"), "A multi-digit superscript was not normalized.") ||
        !check(formats_as("a^²", "a^2"), "A superscript after a caret was not normalized.") ||
        !check(formats_as("Σ|ε|∅", "Σ|ε|∅"), "Special literal formatting changed."))
    {
        return 1;
    }

    const regex::Expression inspected = parse("a(B|a)^2");
    if (!check(regex::inspection::contains_terminal(inspected), "Terminal detection failed.") ||
        !check(
            regex::inspection::terminals(inspected) == std::set<char>({'B', 'a'}),
            "Terminal collection failed."
        ) ||
        !check(regex::inspection::node_count(parse("ab")) == 3, "Node counting failed."))
    {
        return 1;
    }

    if (!check(
            std::holds_alternative<regex::Epsilon>(regex::make_concatenation({})),
            "Empty concatenation did not become epsilon."
        ) ||
        !check(
            std::holds_alternative<regex::EmptySet>(regex::make_alternation({})),
            "Empty alternation did not become the empty set."
        ) ||
        !check(
            std::holds_alternative<std::shared_ptr<const regex::Complement>>(
                regex::make_intersection({})
            ),
            "Empty intersection did not become the universal language."
        ) ||
        !check(
            regex::inspection::structurally_equal(
                regex::make_concatenation({regex::make_terminal('a')}), regex::make_terminal('a')
            ),
            "A singleton aggregate was not collapsed."
        ))
    {
        return 1;
    }

    bool rejected_invalid_terminal = false;
    try
    {
        (void)regex::make_terminal('_');
    }
    catch (const std::invalid_argument&)
    {
        rejected_invalid_terminal = true;
    }
    if (!check(rejected_invalid_terminal, "The model accepted a non-alphanumeric terminal.") ||
        !check(!regex::parsing::parse("_").success(), "The parser accepted punctuation.") ||
        !check(!regex::parsing::parse("ä").success(), "The parser accepted a Unicode terminal.") ||
        !check(!regex::parsing::parse("a|").success(), "The parser accepted malformed input.") ||
        !check(!regex::parsing::parse("|a").success(), "The parser accepted a leading alternation.") ||
        !check(!regex::parsing::parse("a||").success(), "The parser accepted a trailing alternation alias.") ||
        !check(
            !regex::parsing::parse("a&").success(), "The parser accepted malformed intersection."
        ) ||
        !check(
            !regex::parsing::parse("&a").success(), "The parser accepted a leading intersection."
        ) ||
        !check(
            !regex::parsing::parse("a&&").success(),
            "The parser accepted a trailing intersection alias."
        ) ||
        !check(
            parse_error_contains("a^", "exponent"),
            "Missing finite exponent did not produce an actionable diagnostic."
        ) ||
        !check(
            parse_error_contains("a^ω", "omega"),
            "Omega power in finite mode did not produce an actionable diagnostic."
        ) ||
        !check(
            regex::parsing::parse("a^4294967295").success(),
            "The parser rejected the largest supported exponent."
        ) ||
        !check(
            !regex::parsing::parse("a^4294967296").success(),
            "The parser accepted an overflowing exponent."
        ) ||
    !check(regex::parsing::parse("a && b || c").success(), "Boolean aliases failed.") ||
    !check(
        regex::parsing::normalize_input(" a & & b | | c ") == "a&b|c",
        "Input normalization changed."
    ) ||
    !check(
        regex::parsing::normalize_input(" a ⁰¹²³⁴⁵⁶⁷⁸⁹ ") == "a^0123456789",
        "Superscript digit normalization changed."
    ))
    {
        return 1;
    }

    const regex::parsing::ParseResult positioned = regex::parsing::parse("a |\n  #");
    if (!positioned.error.has_value())
    {
        check(false, "Invalid input did not produce a diagnostic.");
        return 1;
    }
    const regex::parsing::ParseError& positioned_error = positioned.error.value();
    if (!check(positioned_error.byte_offset == 6, "Diagnostic byte offset was normalized.") ||
        !check(positioned_error.line == 2, "Diagnostic line was incorrect.") ||
        !check(positioned_error.column == 3, "Diagnostic column was incorrect."))
    {
        return 1;
    }

            using regex::simplification::normalize;
        const regex::Expression normalized = normalize(parse("((a|∅)|a)(εb)"));
        if (!check(regex::formatting::format(normalized) == "ab", "Basic identities changed.") ||
            !check(
                regex::inspection::structurally_equal(normalized, normalize(normalized)),
                "Normalization is not idempotent."
            ) ||
            !check(
                regex::formatting::format(normalize(parse("a+"))) == "a+",
                "Unconditional normalization expanded plus."
            ) ||
            !check(
                regex::formatting::format(normalize(parse("a^2"))) == "a^2",
                "Unconditional normalization expanded power."
            ) ||
            !check(
                regex::formatting::format(normalize(parse("a&a"))) == "a",
                "Intersection deduplication failed."
            ) ||
            !check(
                regex::formatting::format(normalize(parse("ε|a*"))) == "a*",
                "Epsilon was not absorbed by Kleene star."
            ) ||
            !check(
                regex::formatting::format(normalize(parse("a*|ε"))) == "a*",
                "Epsilon was not absorbed by Kleene star in reversed order."
            ) ||
            !check(
                regex::formatting::format(normalize(parse("ε|a+"))) == "a*",
                "Epsilon plus one-or-more repetition did not normalize to Kleene star."
            ) ||
            !check(
                regex::formatting::format(normalize(parse("a+|ε"))) == "a*",
                "One-or-more repetition plus epsilon did not normalize to Kleene star."
            ) ||
            !check(
                regex::formatting::format(normalize(parse("a+|a*"))) == "a*",
                "Plus was not absorbed by the matching Kleene star."
            ) ||
            !check(
                regex::formatting::format(normalize(parse("a*|a+"))) == "a*",
                "Plus was not absorbed by the matching Kleene star in reversed order."
            ) ||
            !check(
                regex::formatting::format(normalize(parse("ε|(ab)+"))) == "(ab)*",
                "Epsilon plus grouped one-or-more repetition did not normalize to grouped Kleene star."
            ) ||
            !check(
                regex::formatting::format(normalize(parse("(ab)+|(ab)*"))) == "(ab)*",
                "Grouped plus was not absorbed by grouped Kleene star."
            ) ||
            !check(
                regex::formatting::format(normalize(parse("ε|a+"))) != "a+",
                "Epsilon plus one-or-more repetition was incorrectly normalized to plus."
            ))
        {
            return 1;
        }

    const regex::Expression double_complement =
           regex::make_complement(regex::make_complement(regex::make_terminal('a')));
    const regex::Expression triple_complement =
        regex::make_complement(
            regex::make_complement(regex::make_complement(regex::make_terminal('a')))
        );
    if (!check(
            regex::inspection::structurally_equal(double_complement, regex::make_terminal('a')),
            "Double complement was not simplified during AST construction."
        ) ||
        !check(
            regex::formatting::format(triple_complement) == "!a",
            "Triple complement was not simplified to a single complement."
        ) ||
        !check(
            regex::formatting::format(normalize(triple_complement)) == "!a",
            "Complement normalization changed construction-time parity simplification."
        ))
    {
        return 1;
    }

    regex::generation::Config generator_config;
    regex::generation::Generator first(12345);
    regex::generation::Generator second(12345);
    for (int sample = 0; sample < 20; ++sample)
    {
        const std::string first_text = first.generate_string(generator_config);
        const std::string second_text = second.generate_string(generator_config);
        if (!check(first_text == second_text, "Equal seeds produced different expressions.") ||
            !check(regex::parsing::parse(first_text).success(), "Generated output did not parse."))
        {
            return 1;
        }
    }

    regex::generation::Config fallback;
    fallback.stop_weight = 0;
    fallback.continue_weight = 20;
    fallback.empty_set_weight = 0;
    fallback.epsilon_weight = 0;
    fallback.letter_weight = 0;
    fallback.kleene_star_weight = 0;
    fallback.plus_weight = 0;
    fallback.complement_weight = 0;
    fallback.concatenation_weight = 0;
    fallback.alternation_weight = 0;
    fallback.intersection_weight = 0;
    regex::generation::Generator fallback_generator(7);
    const regex::Expression fallback_expression = fallback_generator.generate(fallback);
    if (!check(
            std::holds_alternative<regex::Terminal>(fallback_expression),
            "Zero generator weights did not use the safe letter fallback."
        ))
    {
        return 1;
    }

    regex::generation::Config deepest;
    deepest.stop_weight = 0;
    deepest.continue_weight = 1;
    deepest.empty_set_weight = 0;
    deepest.epsilon_weight = 0;
    deepest.letter_weight = 1;
    deepest.kleene_star_weight = 0;
    deepest.plus_weight = 0;
    deepest.complement_weight = 0;
    deepest.concatenation_weight = 1;
    deepest.alternation_weight = 0;
    deepest.intersection_weight = 0;
    deepest.max_depth = 100;
    regex::generation::Generator depth_generator(11);
    if (!check(
            regex::inspection::node_count(depth_generator.generate(deepest)) == 2047,
            "Generator depth sanitization changed."
        ))
    {
        return 1;
    }

    return 0;
}
