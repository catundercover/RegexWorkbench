// Defines the immutable-node omega-regular-expression abstract syntax tree.
#pragma once

#include "regex/model/Expression.hpp"

#include <memory>
#include <variant>
#include <vector>

namespace regex::omega
{
    struct Alternation;
    struct Intersection;
    struct Concatenation;
    struct Complement;
    struct OmegaPower;

    // Matches no infinite word.
    struct EmptySet
    {
        // All omega empty-set nodes are structurally equal.
        bool operator==(const EmptySet&) const = default;
    };

    struct UniversalSet {};

    // Tagged omega expression; recursive nodes use immutable shared ownership.
    using Expression = std::variant<
        EmptySet,
        UniversalSet,
        std::shared_ptr<const Alternation>,
        std::shared_ptr<const Intersection>,
        std::shared_ptr<const Concatenation>,
        std::shared_ptr<const Complement>,
        std::shared_ptr<const OmegaPower>>;

    // Matches an infinite word accepted by any alternative.
    struct Alternation
    {
        std::vector<Expression> alternatives;
    };

    // Matches an infinite word accepted by every operand.
    struct Intersection
    {
        std::vector<Expression> operands;
    };

    // Matches a finite prefix followed by an omega-language suffix.
    struct Concatenation
    {
        regex::Expression prefix;
        Expression suffix;
    };

    // Matches the complement over the active infinite-word alphabet.
    struct Complement
    {
        Expression expression;
    };

    // Matches an infinite concatenation of non-empty words from a finite language.
    struct OmegaPower
    {
        regex::Expression expression;
    };

    // Constructs an alternation, collapsing zero or one alternatives.
    [[nodiscard]] Expression make_alternation(std::vector<Expression> alternatives);
    // Constructs an intersection, collapsing zero or one operands.
    [[nodiscard]] Expression make_intersection(std::vector<Expression> operands);
    // Constructs a finite-prefix/omega-suffix concatenation.
    [[nodiscard]] Expression make_concatenation(regex::Expression prefix, Expression suffix);
    // Constructs an omega-language complement.
    [[nodiscard]] Expression make_complement(Expression expression);
    // Constructs infinite repetition of a finite-language expression.
    [[nodiscard]] Expression make_omega_power(regex::Expression expression);
}
