// Verifies parser behavior for omega regular expressions.
#include "automata/conversion/OmegaRegexAlphabet.hpp"
#include "regex/formatting/OmegaRegexFormatter.hpp"
#include "regex/model/OmegaExpression.hpp"
#include "regex/parsing/OmegaRegexParser.hpp"
#include "support/TestSupport.hpp"

#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

namespace
{
    // Parses a required omega expression used by a language test.
    regex::omega::Expression parse_omega(const std::string_view input)
    {
        regex::parsing::omega::ParseResult result = regex::parsing::omega::parse(input);
        if (!result.expression.has_value() || result.error.has_value())
        {
            throw std::runtime_error("Test omega expression did not parse: " + std::string(input));
        }
        return std::move(result.expression).value();
    }

    // Returns whether the supplied omega expression parses successfully.
    bool parses(const std::string_view input)
    {
        return regex::parsing::omega::parse(input).success();
    }

    // Returns whether the supplied omega expression is rejected.
    bool rejects(const std::string_view input)
    {
        return !regex::parsing::omega::parse(input).success();
    }

    // Checks that omega parsing fails with a diagnostic containing expected text.
    bool omega_parse_error_contains(const std::string_view input, const std::string_view expected)
    {
        const regex::parsing::omega::ParseResult result = regex::parsing::omega::parse(input);
        return !result.success() && result.error.has_value() &&
               result.error->message.find(expected) != std::string::npos;
    }

    // Checks whether an omega expression parses and reports a stable failure message.
    bool check_parses(const std::string_view input)
    {
        return test_support::check(
            parses(input), "Valid omega expression was rejected: " + std::string(input)
        );
    }

    // Checks whether an omega expression is rejected and reports a stable failure message.
    bool check_rejects(const std::string_view input)
    {
        return test_support::check(
            rejects(input), "Invalid omega expression was accepted: " + std::string(input)
        );
    }

    // Returns whether an omega expression is a finite-prefix/omega-suffix concatenation.
    bool is_omega_concatenation(const regex::omega::Expression& expression)
    {
        return std::holds_alternative<std::shared_ptr<const regex::omega::Concatenation>>(expression);
    }

    // Returns whether an omega expression is an omega power.
    bool is_omega_power(const regex::omega::Expression& expression)
    {
        return std::holds_alternative<std::shared_ptr<const regex::omega::OmegaPower>>(expression);
    }

    // Returns whether an omega expression is an omega complement.
    bool is_omega_complement(const regex::omega::Expression& expression)
    {
        return std::holds_alternative<std::shared_ptr<const regex::omega::Complement>>(expression);
    }
}

// Runs omega parser, normalization, and alphabet checks.
int main()
{
    constexpr std::array<std::string_view, 31> Valid{
        "a^ω",
        "(a*)^ω",
        "(ab)^ω",
        "ab^ω",
        "a*b^ω",
        "(a|b)^ω",
        "((a|b)*c)^ω",
        "ε^ω",
        "∅^ω",
        "∅",
        "!∅",
        "!(a^ω)",
        "a^ω|b^ω",
        "a^ω&b^ω",
        "a*(bc)^ω",
        "a*(b^ω|c^ω)",
        "!!a^ω",
        "(!a)^ω",
        "Σ^ω",
        "~Σ^ω",
        "aΣ^ω",
        "abΣ^ω",
        "abc^ω",
        "a(b^ω)",
        "a(b^ω|c^ω)",
        "(a|b)c^ω",
        "a^2b^ω",
        "(ab)^2c^ω",
        "~(a^ω|b^ω)",
        "!(a^ω&b^ω)",
        "((a^ω))"
    };

    for (const std::string_view input : Valid)
    {
        if (!check_parses(input))
        {
            return 1;
        }
    }

    constexpr std::array<std::string_view, 22> Invalid{
        "",
        "a",
        "a*",
        "ε",
        "Σ",
        "a|b^ω",
        "a&b^ω",
        "a^ωb",
        "a^ωb^ω",
        "(a^ω)^ω",
        "(a^ω)*",
        "(a^ω)^2",
        "ω",
        "^ω",
        "a^",
        "a^^ω",
        "a^ω|",
        "a^ω&",
        "|a^ω",
        "&a^ω",
        "(a^ω",
        "a^ω)"
    };

    for (const std::string_view input : Invalid)
    {
        if (!check_rejects(input))
        {
            return 1;
        }
    }

    if (!test_support::check(
            omega_parse_error_contains("a", "finite expressions must be followed"),
            "Finite-only omega input did not produce an actionable diagnostic."
        ) ||
        !test_support::check(
            omega_parse_error_contains("a*", "finite expressions must be followed"),
            "Finite-only repeated omega input did not produce an actionable diagnostic."
        ) ||
        !test_support::check(
            omega_parse_error_contains("ε", "finite expressions must be followed"),
            "Finite-only epsilon omega input did not produce an actionable diagnostic."
        ) ||
        !test_support::check(
            omega_parse_error_contains("a^ωb", "cannot concatenate"),
            "Trailing expression after omega power did not produce an actionable diagnostic."
        ))
    {
        return 1;
    }

    const regex::omega::Expression empty = parse_omega("∅");
    if (!test_support::check(
            std::holds_alternative<regex::omega::EmptySet>(empty),
            "Omega empty-set literal did not produce an omega empty-set node."
        ))
    {
        return 1;
    }

    const regex::omega::Expression universal = parse_omega("Σ^ω");
    if (!test_support::check(
            std::holds_alternative<regex::omega::UniversalSet>(universal),
            "Sigma omega literal did not produce an omega universal-set node."
        ))
    {
        return 1;
    }

    const regex::omega::Expression power = parse_omega("(a|b)^ω");
    const automata::Alphabet alphabet = automata::conversion::alphabet_of(power);
    if (!test_support::check(
            alphabet == automata::Alphabet{'a', 'b'},
            "Omega alphabet extraction did not collect the finite operand terminals."
        ))
    {
        return 1;
    }

    if (!test_support::check(
            regex::formatting::omega::format(power) == "(a|b)^ω",
            "Formatting lost the grouping around an omega-power operand."
        ) ||
        !test_support::check(
            parses(regex::formatting::omega::format(parse_omega("a*(b^ω|c^ω)"))),
            "A formatted prefixed omega expression did not parse again."
        ) ||
        !test_support::check(
            regex::formatting::omega::format(parse_omega("(a|b)c^ω")) == "(a|b)c^ω",
            "Formatting lost the grouping around a finite alternation prefix."
        ))
    {
        return 1;
    }

    const regex::omega::Expression epsilon_power = parse_omega("ε^ω");
    if (!test_support::check(
            std::holds_alternative<std::shared_ptr<const regex::omega::OmegaPower>>(epsilon_power),
            "Epsilon omega power did not produce an omega-power node."
        ))
    {
        return 1;
    }

    const regex::omega::Expression prefixed_power = parse_omega("ab^ω");
    if (!test_support::check(
            is_omega_concatenation(prefixed_power),
            "Expression 'ab^ω' was not parsed as a finite prefix followed by an omega suffix."
        ))
    {
        return 1;
    }

    const regex::omega::Expression grouped_power = parse_omega("(ab)^ω");
    if (!test_support::check(
            is_omega_power(grouped_power),
            "Expression '(ab)^ω' was not parsed as one omega power over the grouped finite operand."
        ))
    {
        return 1;
    }

    const regex::omega::Expression complemented_power = parse_omega("!a^ω");
    if (!test_support::check(
            is_omega_complement(complemented_power),
            "Expression '!a^ω' was not parsed as an omega complement."
        ))
    {
        return 1;
    }

    const regex::omega::Expression finite_complement_power = parse_omega("(!a)^ω");
    if (!test_support::check(
            is_omega_power(finite_complement_power),
            "Expression '(!a)^ω' was not parsed as omega power over a finite complement operand."
        ))
    {
        return 1;
    }

    const regex::omega::Expression prefixed_group = parse_omega("a(b^ω|c^ω)");
    if (!test_support::check(
            is_omega_concatenation(prefixed_group),
            "Expression 'a(b^ω|c^ω)' was not parsed as finite-prefix omega concatenation."
        ))
    {
        return 1;
    }

    std::string long_valid_prefix(256, 'a');
    long_valid_prefix += "b^ω";
    if (!test_support::check(
            parses(long_valid_prefix),
            "Long finite-prefix omega expression was rejected."
        ))
    {
        return 1;
    }

    const std::string long_finite_only(512, 'a');
    if (!test_support::check(
            rejects(long_finite_only),
            "Long finite-only input was accepted as an omega expression."
        ))
    {
        return 1;
    }

    const regex::parsing::omega::ParseResult positioned =
        regex::parsing::omega::parse("a^ω |\n  #");
    if (!test_support::check(
            positioned.error.has_value(), "Invalid omega input did not produce a diagnostic."
        ))
    {
        return 1;
    }

    return 0;
}