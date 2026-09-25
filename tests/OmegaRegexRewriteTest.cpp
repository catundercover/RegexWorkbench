#include "app/operations/OperationExecutor.hpp"
#include "app/operations/OperationModels.hpp"
#include "automata/analysis/OmegaRegexComparison.hpp"
#include "automata/rewrite/OmegaRegexRewriter.hpp"
#include "regex/formatting/OmegaRegexFormatter.hpp"
#include "regex/inspection/OmegaExpressionInspection.hpp"
#include "regex/model/OmegaExpression.hpp"
#include "regex/parsing/OmegaRegexParser.hpp"
#include "regex/simplification/OmegaRegexSimplifier.hpp"
#include "support/TestSupport.hpp"

#include <array>
#include <string>
#include <string_view>
#include <variant>

namespace
{
    regex::omega::Expression parse_omega(const std::string_view text)
    {
        return test_support::parse_omega_expression(text);
    }

    bool omega_equivalent(
        const std::string_view left,
        const std::string_view right,
        const automata::Alphabet& alphabet = {'a', 'b'}
    )
    {
        const automata::analysis::OmegaRegexComparison comparison =
            automata::analysis::compare(parse_omega(left), parse_omega(right), alphabet);

        return test_support::check(
            comparison.relation == automata::analysis::LanguageRelation::Equivalent,
            std::string(left) + " and " + std::string(right) + " are not omega-equivalent."
        );
    }

    bool rewrites_to(
        const std::string_view input,
        const automata::rewrite::omega::Options& options,
        const automata::Alphabet& alphabet,
        const std::string_view expected
    )
    {
        const std::string rewritten =
            automata::rewrite::omega::apply(parse_omega(input), options, alphabet);

        return test_support::check(
            rewritten == expected,
            std::string(input) + " rewrote to '" + rewritten + "', expected '" +
                std::string(expected) + "'."
        );
    }

    bool rewrites_equivalently(
        const std::string_view input,
        const automata::rewrite::omega::Options& options,
        const automata::Alphabet& alphabet = {'a', 'b'}
    )
    {
        const std::string rewritten =
            automata::rewrite::omega::apply(parse_omega(input), options, alphabet);

        const regex::parsing::omega::ParseResult parsed = regex::parsing::omega::parse(rewritten);

        if (!test_support::check(
                parsed.expression.has_value() && !parsed.error.has_value(),
                std::string(input) + " rewrote to an unparsable omega regex: " + rewritten
            ))
        {
            return false;
        }

        return omega_equivalent(input, rewritten, alphabet);
    }

    bool rewrites_without(
        const std::string_view input,
        const automata::rewrite::omega::Options& options,
        const automata::Alphabet& alphabet,
        const char removed_operator
    )
    {
        const std::string rewritten =
            automata::rewrite::omega::apply(parse_omega(input), options, alphabet);

        return test_support::check(
                   rewritten.find(removed_operator) == std::string::npos,
                   std::string(input) +
                       " still contains the removed operator after rewriting: " + rewritten
               ) &&
               omega_equivalent(input, rewritten, alphabet);
    }
}

int main()
{
    automata::rewrite::omega::Options keep;

    if (!rewrites_to("ε^ω", keep, {'a'}, "∅") || !rewrites_to("∅^ω", keep, {'a'}, "∅"))
    {
        return 1;
    }

    if (!rewrites_to("(a|∅)^ω", keep, {'a'}, "a^ω") || !rewrites_to("εa^ω", keep, {'a'}, "a^ω") ||
        !rewrites_to("!!a^ω", keep, {'a'}, "a^ω") || !rewrites_to("a^ω|∅", keep, {'a'}, "a^ω") ||
        !rewrites_to("a^ω&a^ω", keep, {'a'}, "a^ω"))
    {
        return 1;
    }

    if (!test_support::check(
            regex::inspection::omega::structurally_equal(
                regex::simplification::omega::normalize(parse_omega("(a|∅)^ω")),
                regex::simplification::omega::normalize(
                    regex::simplification::omega::normalize(parse_omega("(a|∅)^ω"))
                )
            ),
            "Omega normalization is not idempotent."
        ))
    {
        return 1;
    }

    automata::rewrite::omega::Options syntax_only;
    syntax_only.remove_plus = true;
    syntax_only.remove_power = true;
    syntax_only.remove_any_symbol = true;

    if (!rewrites_to("(a+)^ω", syntax_only, {'a'}, "(aa*)^ω") ||
        !rewrites_to("(a^3)^ω", syntax_only, {'a'}, "(aaa)^ω") ||
        !rewrites_to("Σ^ω", syntax_only, {'a', 'b'}, "(a|b)^ω") ||
        !rewrites_to("!∅", syntax_only, {'a', 'b'}, "(a|b)^ω") ||
        !rewrites_to("a+(b^2)^ω", syntax_only, {'a', 'b'}, "aa*(bb)^ω"))
    {
        return 1;
    }

    /*
     * These checks should not require omega automata construction: only finite
     * operands are expanded syntactically, and true omega operators are kept.
     */
    if (!rewrites_to("!(a+)^ω", syntax_only, {'a'}, "!(aa*)^ω") ||
        !rewrites_to("(a+)^ω&b^ω", syntax_only, {'a', 'b'}, "(aa*)^ω&b^ω"))
    {
        return 1;
    }

    automata::rewrite::omega::Options remove_omega_operators = syntax_only;
    remove_omega_operators.remove_complement = true;
    remove_omega_operators.remove_intersection = true;

    constexpr std::array<std::string_view, 5> EquivalentRewriteFixtures{
        "(a|∅)^ω", "εa^ω", "(a+)^ω", "(a^2|b)^ω", "a^ω|b^ω"
    };

    for (const std::string_view fixture : EquivalentRewriteFixtures)
    {
        if (!rewrites_equivalently(fixture, syntax_only))
        {
            return 1;
        }
    }

    /*
     * Keep the automata-backed cases conservative. They verify that the path
     * returns parseable, equivalent omega regexes for simple formulas. If the
     * implementation deliberately rejects symbolic BDD labels, this still catches
     * regressions in the supported subset.
     */
    if (!rewrites_equivalently("a^ω&a^ω", remove_omega_operators, {'a'}) ||
        !rewrites_equivalently("!!a^ω", remove_omega_operators, {'a'}) ||
        !rewrites_equivalently("!a^ω", remove_omega_operators, {'a', 'b'}) ||
        !rewrites_equivalently("(a|b)^ω&!b^ω", remove_omega_operators, {'a', 'b'}) ||
        !rewrites_without("(!a)^ω", remove_omega_operators, {'a', 'b'}, '!') ||
        !rewrites_without("(a&b)^ω", remove_omega_operators, {'a', 'b'}, '&'))
    {
        return 1;
    }

    app::operations::RewriteRequest request;
    request.regex = "(a+)^ω";
    request.flavor = app::operations::ExpressionFlavor::OmegaRegex;
    request.extra_alphabet = "a";
    request.remove_plus = true;

    const app::operations::OperationResult result = app::operations::execute(request);
    const auto* rewritten = std::get_if<app::operations::RewriteResult>(&result);

    if (!test_support::check(
            rewritten != nullptr, "Omega rewrite operation did not return a rewrite result."
        ) ||
        !test_support::check(
            rewritten->rewritten_regex == "(aa*)^ω",
            "Omega rewrite operation returned unexpected text: " +
                (rewritten != nullptr ? rewritten->rewritten_regex : std::string{})
        ))
    {
        return 1;
    }

    app::operations::RewriteRequest invalid_as_omega;
    invalid_as_omega.regex = "a+";
    invalid_as_omega.flavor = app::operations::ExpressionFlavor::OmegaRegex;
    invalid_as_omega.extra_alphabet = "a";

    const app::operations::OperationResult invalid_result =
        app::operations::execute(invalid_as_omega);

    if (!test_support::check(
            std::holds_alternative<app::operations::OperationFailure>(invalid_result),
            "Finite regex syntax unexpectedly rewrote in omega mode."
        ))
    {
        return 1;
    }

    return 0;
}
