// Property-tests formatting, simplification, and regex language preservation.
#include "regex/formatting/RegexFormatter.hpp"
#include "regex/generation/RandomRegexGenerator.hpp"
#include "regex/inspection/ExpressionInspection.hpp"
#include "regex/simplification/RegexSimplifier.hpp"
#include "support/TestSupport.hpp"

#include <array>
#include <cstdint>
#include <string>

namespace
{
    // Checks that formatting and reparsing preserve expression structure.
    bool check_round_trip(const regex::Expression& expression, const std::string& label)
    {
        const std::string formatted = regex::formatting::format(expression);
        const regex::Expression reparsed = test_support::parse_expression(formatted);
        return test_support::check(
            regex::inspection::structurally_equal(expression, reparsed),
            label + " did not survive format/parse round-tripping: " + formatted
        );
    }
}

// Runs deterministic random-generation and normalization properties.
int main()
{
    constexpr std::array<std::string_view, 22> Expressions{
        "∅",
        "ε",
        "Σ",
        "a",
        "a|b&c",
        "(a|b)c",
        "!(a|b)",
        "(a|ε)*",
        "a+",
        "(ab)^3",
        "a&(b|c)",
        "((a|b)*)+",
        "!(a&b)",
        "Σ^2",
        "a|(b|(c|d))",
        "((a))",
        "ε|a*",
        "ε|a+",
        "a+|a*",
        "a*|a+",
        "ε|(ab)+",
        "(ab)+|(ab)*"
    };

    for (const std::string_view text : Expressions)
    {
        const regex::Expression parsed = test_support::parse_expression(text);
        if (!check_round_trip(parsed, std::string(text)))
        {
            return 1;
        }

        const regex::Expression normalized = regex::simplification::normalize(parsed);
        if (!test_support::check(
                regex::inspection::structurally_equal(
                    normalized, regex::simplification::normalize(normalized)
                ),
                std::string(text) + " did not normalize idempotently."
            ) ||
            !check_round_trip(normalized, std::string(text) + " after normalization"))
        {
            return 1;
        }
    }

    regex::generation::Config config;
    config.max_depth = 5;
    for (std::uint32_t seed = 0; seed < 64; ++seed)
    {
        regex::generation::Generator generator(seed);
        for (int sample = 0; sample < 3; ++sample)
        {
            const regex::Expression generated = generator.generate(config);
            if (!check_round_trip(
                    generated, "Generated expression for seed " + std::to_string(seed)
                ))
            {
                return 1;
            }

            const regex::Expression normalized = regex::simplification::normalize(generated);
            if (!test_support::check(
                    regex::inspection::structurally_equal(
                        normalized, regex::simplification::normalize(normalized)
                    ),
                    "Generated expression did not normalize idempotently for seed " +
                        std::to_string(seed)
                ))
            {
                return 1;
            }
        }
    }

    return 0;
}
