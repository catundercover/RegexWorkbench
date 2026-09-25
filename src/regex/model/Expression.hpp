// Defines the immutable-node regular-expression abstract syntax tree.
#pragma once

#include <memory>
#include <variant>
#include <vector>

namespace regex
{
    struct Alternation;
    struct Intersection;
    struct Concatenation;
    struct KleeneStar;
    struct Plus;
    struct Complement;
    struct Power;

    // Matches one specific ASCII letter or digit.
    struct Terminal
    {
        // Constructs a terminal and rejects unsupported characters.
        explicit Terminal(char value);

        char value;

        // Compares terminal symbols.
        bool operator==(const Terminal&) const = default;
    };

    // Matches the empty word.
    struct Epsilon
    {
        // All epsilon nodes are structurally equal.
        bool operator==(const Epsilon&) const = default;
    };

    // Matches no word.
    struct EmptySet
    {
        // All empty-set nodes are structurally equal.
        bool operator==(const EmptySet&) const = default;
    };

    // Matches exactly one symbol from the active alphabet (Sigma), not Sigma-star.
    struct AnySymbol
    {
        // All any-symbol nodes are structurally equal.
        bool operator==(const AnySymbol&) const = default;
    };

    // Tagged regular-expression node; recursive nodes use immutable shared ownership.
    using Expression = std::variant<
        Terminal,
        Epsilon,
        EmptySet,
        AnySymbol,
        std::shared_ptr<const Alternation>,
        std::shared_ptr<const Concatenation>,
        std::shared_ptr<const KleeneStar>,
        std::shared_ptr<const Plus>,
        std::shared_ptr<const Complement>,
        std::shared_ptr<const Intersection>,
        std::shared_ptr<const Power>>;

    // Matches any one of its operand expressions.
    struct Alternation
    {
        std::vector<Expression> alternatives;
    };

    // Matches words accepted by every operand expression.
    struct Intersection
    {
        std::vector<Expression> operands;
    };

    // Matches its parts consecutively from left to right.
    struct Concatenation
    {
        std::vector<Expression> parts;
    };

    // Matches zero or more repetitions of its operand.
    struct KleeneStar
    {
        Expression expression;
    };

    // Matches one or more repetitions of its operand.
    struct Plus
    {
        Expression expression;
    };

    // Matches the complement of its operand over the active alphabet.
    struct Complement
    {
        Expression expression;
    };

    // Matches exactly `exponent` concatenated copies of its operand.
    struct Power
    {
        Expression expression;
        unsigned int exponent;
    };

    // Returns whether a character is a supported terminal symbol.
    [[nodiscard]] bool is_ascii_terminal(char value);

    // Constructs a validated terminal expression.
    [[nodiscard]] Expression make_terminal(char value);
    // Constructs an alternation, collapsing zero or one operands.
    [[nodiscard]] Expression make_alternation(std::vector<Expression> alternatives);
    // Constructs an intersection, collapsing zero or one operands.
    [[nodiscard]] Expression make_intersection(std::vector<Expression> operands);
    // Constructs a concatenation, collapsing zero or one parts.
    [[nodiscard]] Expression make_concatenation(std::vector<Expression> parts);
    // Constructs a Kleene-star expression.
    [[nodiscard]] Expression make_star(Expression expression);
    // Constructs a one-or-more repetition expression.
    [[nodiscard]] Expression make_plus(Expression expression);
    // Constructs a complement expression.
    [[nodiscard]] Expression make_complement(Expression expression);
    // Constructs an exact-repetition expression.
    [[nodiscard]] Expression make_power(Expression expression, unsigned int exponent);
}
