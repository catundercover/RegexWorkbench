// Defines the lexy grammar and AST callbacks for regular expressions.
#pragma once

#include "regex/model/Expression.hpp"

#include <lexy/callback.hpp>
#include <lexy/callback/container.hpp>
#include <lexy/dsl.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/input/string_input.hpp>
#include <string>
#include <utility>
#include <vector>

namespace regex::parsing::detail
{
    namespace dsl = lexy::dsl;

    // Semantic marker emitted for the Kleene-star postfix operator.
    struct StarOperator
    {
    };
    // Semantic marker emitted for the one-or-more postfix operator.
    struct PlusOperator
    {
    };
    // Semantic marker emitted for either complement spelling.
    struct ComplementOperator
    {
    };

    struct Grammar;
    struct Atom;
    struct Repetition;
    struct ComplementExpression;
    struct ConcatenationExpression;
    struct IntersectionExpression;

    // Accepts ASCII letters and digits as terminal code points.
    struct AsciiTerminal
    {
        // Returns whether the supplied code point is a supported terminal.
        constexpr bool operator()(const lexy::code_point code_point) const
        {
            const char32_t value = code_point.value();
            return (value >= U'a' && value <= U'z') || (value >= U'A' && value <= U'Z') ||
                   (value >= U'0' && value <= U'9');
        }
    };

    // Accepts the epsilon, empty-set, and any-symbol Unicode literals.
    struct SpecialLiteral
    {
        // Returns whether the supplied code point is a supported special literal.
        constexpr bool operator()(const lexy::code_point code_point) const
        {
            return code_point.value() == U'\u03B5' || code_point.value() == U'\u2205' ||
                   code_point.value() == U'\u03A3';
        }
    };

    // Parses terminals, special literals, and parenthesized expressions.
    struct Atom
    {
        static constexpr auto rule = []
        {
            const auto special = dsl::capture(dsl::code_point.if_<SpecialLiteral>());
            const auto terminal = dsl::capture(dsl::code_point.if_<AsciiTerminal>());
            const auto parenthesized =
                dsl::brackets(dsl::lit_c<'('>, dsl::lit_c<')'>)(dsl::recurse<Grammar>);
            return special | terminal | parenthesized;
        }();

        static constexpr auto value = lexy::callback<Expression>(
            [](Expression expression) { return expression; },
            [](const auto lexeme)
            {
                const std::string text(lexeme.begin(), lexeme.end());
                if (text == "\xCE\xB5")
                {
                    return Expression{Epsilon{}};
                }
                if (text == "\xE2\x88\x85")
                {
                    return Expression{EmptySet{}};
                }
                if (text == "\xCE\xA3")
                {
                    return Expression{AnySymbol{}};
                }
                return make_terminal(text.front());
            }
        );
    };

    // Parses an atom followed by an optional exact exponent.
    struct PowerExpression
    {
        static constexpr auto rule = []
        {
            const auto exponent_condition = dsl::peek(dsl::lit_c<'^'> + dsl::digit<>);
            const auto exponent = dsl::lit_c<'^'> + dsl::integer<unsigned int>;
            return dsl::recurse<Atom> + dsl::opt(exponent_condition >> exponent);
        }();

        static constexpr auto value = lexy::callback<Expression>(
            [](Expression expression, lexy::nullopt) { return expression; },
            [](Expression expression, const unsigned int exponent)
            { return make_power(std::move(expression), exponent); }
        );
    };

    // Parses an optional star or plus postfix after a power expression.
    struct Repetition
    {
        static constexpr auto rule = []
        {
            const auto postfix = dsl::opt(
                dsl::op<StarOperator>(dsl::lit_c<'*'>) | dsl::op<PlusOperator>(dsl::lit_c<'+'>)
            );
            return dsl::recurse<PowerExpression> + postfix;
        }();

        static constexpr auto value = lexy::callback<Expression>(
            [](Expression expression, lexy::nullopt) { return expression; },
            [](Expression expression, StarOperator) { return make_star(std::move(expression)); },
            [](Expression expression, PlusOperator) { return make_plus(std::move(expression)); }
        );
    };

    // Parses an optional prefix complement before a repetition expression.
    struct ComplementExpression
    {
        static constexpr auto rule = []
        {
            const auto operator_rule = dsl::op<ComplementOperator>(dsl::lit_c<'!'>) |
                                       dsl::op<ComplementOperator>(dsl::lit_c<'~'>);
            const auto complement_start = dsl::peek(dsl::lit_c<'!'> | dsl::lit_c<'~'>);

            return complement_start >>
                       (operator_rule + dsl::recurse<ComplementExpression>) |
                   dsl::else_ >> dsl::recurse<Repetition>;
        }();

        static constexpr auto value = lexy::callback<Expression>(
            [](Expression expression) { return expression; },
            [](ComplementOperator, Expression expression)
            { return make_complement(std::move(expression)); }
        );
    };

    // Parses one or more adjacent expressions as concatenation.
    struct ConcatenationExpression
    {
        static constexpr auto rule = []
        {
            const auto expression_start = dsl::peek(
                dsl::lit_c<'('> | dsl::lit_c<'!'> | dsl::lit_c<'~'> | dsl::lit_cp<U'\u03B5'> |
                dsl::lit_cp<U'\u2205'> | dsl::lit_cp<U'\u03A3'> |
                dsl::code_point.if_<AsciiTerminal>()
            );
            return dsl::list(expression_start >> dsl::recurse<ComplementExpression>);
        }();

        static constexpr auto
            value = lexy::as_list<std::vector<Expression>> >>
                    lexy::callback<Expression>([](std::vector<Expression> parts)
                                               { return make_concatenation(std::move(parts)); });
    };

    // Parses ampersand-separated concatenations as intersection.
    struct IntersectionExpression
    {
        static constexpr auto rule =
            dsl::list(dsl::recurse<ConcatenationExpression>, dsl::sep(dsl::lit_c<'&'>));

        static constexpr auto
            value = lexy::as_list<std::vector<Expression>> >>
                    lexy::callback<Expression>([](std::vector<Expression> operands)
                                               { return make_intersection(std::move(operands)); });
    };

    // Parses pipe-separated intersections as the complete expression grammar.
    struct Grammar
    {
        static constexpr auto rule =
            dsl::list(dsl::recurse<IntersectionExpression>, dsl::sep(dsl::lit_c<'|'>));

        static constexpr auto value =
            lexy::as_list<std::vector<Expression>> >>
            lexy::callback<Expression>([](std::vector<Expression> alternatives)
                                       { return make_alternation(std::move(alternatives)); });
    };

    // Requires a complete grammar match followed by end of input.
    struct Root
    {
        static constexpr auto rule = dsl::recurse<Grammar> + dsl::eof;
        static constexpr auto value = lexy::forward<Expression>;
    };

    // Exposes the finite regular-expression grammar for parsers that embed it.
    struct FiniteExpressionGrammar
    {
        static constexpr auto rule = dsl::recurse<Grammar>;
        static constexpr auto value = lexy::forward<Expression>;
    };

    // Exposes a finite repetition expression for omega-power operands.
    struct FiniteRepetitionGrammar
    {
        static constexpr auto rule = dsl::recurse<Repetition>;
        static constexpr auto value = lexy::forward<Expression>;
    };

    // Exposes a finite complement/repetition expression for omega concatenation prefixes.
    struct FiniteComplementGrammar
    {
        static constexpr auto rule = dsl::recurse<ComplementExpression>;
        static constexpr auto value = lexy::forward<Expression>;
    };
}
