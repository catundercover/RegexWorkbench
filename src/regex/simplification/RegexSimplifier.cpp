// Implements recursive language-preserving regex simplification.
#include "regex/simplification/RegexSimplifier.hpp"

#include "regex/inspection/ExpressionInspection.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

namespace regex::simplification
{
    namespace
    {
        // Returns whether an expression is the empty word.
        bool is_epsilon(const Expression& expression)
        {
            return std::holds_alternative<Epsilon>(expression);
        }

        // Returns whether an expression is the empty language.
        bool is_empty(const Expression& expression)
        {
            return std::holds_alternative<EmptySet>(expression);
        }

        // Dereferences a recursive AST node or rejects an invalid null pointer.
        template <typename Node>
        const Node& require_node(const std::shared_ptr<const Node>& node)
        {
            if (!node)
            {
                throw std::invalid_argument("Cannot normalize a null regular-expression node");
            }
            return *node;
        }

        // Returns whether an expression is the universal finite-word language Σ*.
        bool is_universal(const Expression& expression)
        {
            const auto* star = std::get_if<std::shared_ptr<const KleeneStar>>(&expression);
            return star && *star &&
                   std::holds_alternative<AnySymbol>(require_node(*star).expression);
        }

        // Constructs the canonical universal finite-word language Σ*.
        Expression universal()
        {
            return make_star(AnySymbol{});
        }

        bool is_complement_of(const Expression& expression, const Expression& possible_inner)
        {
            const auto* complement = std::get_if<std::shared_ptr<const Complement>>(&expression);
            return complement && *complement &&
                   inspection::structurally_equal(require_node(*complement).expression, possible_inner);
        }

        bool contains_complement_pair(
            const std::vector<Expression>& expressions, const Expression& candidate
        )
        {
            return std::any_of(
                expressions.begin(),
                expressions.end(),
                [&candidate](const Expression& existing)
                {
                    return is_complement_of(existing, candidate) ||
                           is_complement_of(candidate, existing);
                }
            );
        }

        // Appends an expression only when no structurally equal entry exists.
        void add_unique(std::vector<Expression>& expressions, Expression candidate)
        {
            const bool duplicate = std::any_of(
                expressions.begin(),
                expressions.end(),
                [&candidate](const Expression& existing)
                { return inspection::structurally_equal(existing, candidate); }
            );
            if (!duplicate)
            {
                expressions.push_back(std::move(candidate));
            }
        }

        // Normalizes a star and collapses nested or language-trivial repetition.
        Expression normalize_star(const std::shared_ptr<const KleeneStar>& node)
        {
            Expression inner = normalize(require_node(node).expression);
            if (is_empty(inner) || is_epsilon(inner))
            {
                return Epsilon{};
            }
            if (std::holds_alternative<std::shared_ptr<const KleeneStar>>(inner))
            {
                return inner;
            }
            if (const auto* plus = std::get_if<std::shared_ptr<const Plus>>(&inner))
            {
                return normalize(make_star(require_node(*plus).expression));
            }
            return make_star(std::move(inner));
        }

        // Normalizes plus and collapses language-trivial repetition.
        Expression normalize_plus(const std::shared_ptr<const Plus>& node)
        {
            Expression inner = normalize(require_node(node).expression);
            if (is_empty(inner) || is_epsilon(inner))
            {
                return inner;
            }
            if (std::holds_alternative<std::shared_ptr<const KleeneStar>>(inner) ||
                std::holds_alternative<std::shared_ptr<const Plus>>(inner))
            {
                return inner;
            }
            return make_plus(std::move(inner));
        }

        // Flattens concatenation, removes epsilon, and propagates the empty language.
        Expression normalize_concatenation(const std::shared_ptr<const Concatenation>& node)
        {
            std::vector<Expression> parts;
            for (const Expression& part : require_node(node).parts)
            {
                Expression normalized = normalize(part);
                if (is_empty(normalized))
                {
                    return EmptySet{};
                }
                if (is_epsilon(normalized))
                {
                    continue;
                }
                if (const auto* nested =
                        std::get_if<std::shared_ptr<const Concatenation>>(&normalized))
                {
                    const auto& nested_parts = require_node(*nested).parts;
                    parts.insert(parts.end(), nested_parts.begin(), nested_parts.end());
                }
                else
                {
                    parts.push_back(std::move(normalized));
                }
            }
            return make_concatenation(std::move(parts));
        }

        // Flattens alternation, removes duplicates, and applies repetition absorption.
        Expression normalize_alternation(const std::shared_ptr<const Alternation>& node)
        {
            std::vector<Expression> alternatives;
            for (const Expression& alternative : require_node(node).alternatives)
            {
                Expression normalized = normalize(alternative);
                if (is_universal(normalized))
                {
                    return universal();
                }
                if (is_empty(normalized))
                {
                    continue;
                }
                if (contains_complement_pair(alternatives, normalized))
                {
                    return universal();
                }
                if (const auto* nested =
                        std::get_if<std::shared_ptr<const Alternation>>(&normalized))
                {
                    for (const Expression& child : require_node(*nested).alternatives)
                    {
                        if (is_universal(child) || contains_complement_pair(alternatives, child))
                        {
                            return universal();
                        }
                        add_unique(alternatives, child);
                    }
                }
                else
                {
                    add_unique(alternatives, std::move(normalized));
                }
            }

            const bool contains_epsilon =
                std::any_of(alternatives.begin(), alternatives.end(), is_epsilon);

            const bool contains_plus_or_star = std::any_of(
                alternatives.begin(),
                alternatives.end(),
                [](const Expression& alternative)
                {
                    return std::holds_alternative<std::shared_ptr<const Plus>>(alternative) ||
                           std::holds_alternative<std::shared_ptr<const KleeneStar>>(alternative);
                }
            );

            // `ε | r*` is just `r*`, and `ε | r+` is `r*`.
            if (contains_epsilon && contains_plus_or_star)
            {
                std::vector<Expression> rewritten;
                rewritten.reserve(alternatives.size());

                for (Expression& alternative : alternatives)
                {
                    if (is_epsilon(alternative))
                    {
                        continue;
                    }

                    if (const auto* plus = std::get_if<std::shared_ptr<const Plus>>(&alternative))
                    {
                        add_unique(rewritten, normalize(make_star(require_node(*plus).expression)));
                    }
                    else
                    {
                        add_unique(rewritten, std::move(alternative));
                    }
                }

                alternatives = std::move(rewritten);
            }

            // `r | r+`, `r | r*`, and `r+ | r*` add no words beyond the repeated alternative.
            std::vector<bool> absorbed(alternatives.size(), false);
            for (std::size_t candidate = 0; candidate < alternatives.size(); ++candidate)
            {
                for (std::size_t repetition = 0; repetition < alternatives.size(); ++repetition)
                {
                    if (candidate == repetition)
                    {
                        continue;
                    }

                    const Expression* repeated = nullptr;
                    const auto* star =
                        std::get_if<std::shared_ptr<const KleeneStar>>(&alternatives[repetition]);

                    if (const auto* plus =
                            std::get_if<std::shared_ptr<const Plus>>(&alternatives[repetition]))
                    {
                        repeated = &require_node(*plus).expression;
                    }
                    else if (star)
                    {
                        repeated = &require_node(*star).expression;
                    }

                    if (repeated &&
                        inspection::structurally_equal(alternatives[candidate], *repeated))
                    {
                        absorbed[candidate] = true;
                        break;
                    }

                    if (star)
                    {
                        const auto* candidate_plus =
                            std::get_if<std::shared_ptr<const Plus>>(&alternatives[candidate]);

                        if (candidate_plus && inspection::structurally_equal(
                                                  require_node(*candidate_plus).expression,
                                                  require_node(*star).expression
                                              ))
                        {
                            absorbed[candidate] = true;
                            break;
                        }
                    }
                }
            }

            std::vector<Expression> result;
            result.reserve(alternatives.size());
            for (std::size_t index = 0; index < alternatives.size(); ++index)
            {
                if (!absorbed[index])
                {
                    result.push_back(std::move(alternatives[index]));
                }
            }
            return make_alternation(std::move(result));
        }

        // Flattens intersection, removes duplicates, and propagates the empty language.
        Expression normalize_intersection(const std::shared_ptr<const Intersection>& node)
        {
            std::vector<Expression> operands;
            for (const Expression& operand : require_node(node).operands)
            {
                Expression normalized = normalize(operand);
                if (is_empty(normalized))
                {
                    return EmptySet{};
                }
                if (is_universal(normalized))
                {
                    continue;
                }
                if (contains_complement_pair(operands, normalized))
                {
                    return EmptySet{};
                }
                if (const auto* nested =
                            std::get_if<std::shared_ptr<const Intersection>>(&normalized))
                {
                    for (const Expression& child : require_node(*nested).operands)
                    {
                        if (is_empty(child) || contains_complement_pair(operands, child))
                        {
                            return EmptySet{};
                        }
                        if (!is_universal(child))
                        {
                            add_unique(operands, child);
                        }
                    }
                }
                else
                {
                    add_unique(operands, std::move(normalized));
                }
            }
            return make_intersection(std::move(operands));
        }
    }

    Expression normalize(const Expression& expression)
    {
        if (std::holds_alternative<Terminal>(expression) ||
            std::holds_alternative<Epsilon>(expression) ||
            std::holds_alternative<EmptySet>(expression) ||
            std::holds_alternative<AnySymbol>(expression))
        {
            return expression;
        }
        if (const auto* star = std::get_if<std::shared_ptr<const KleeneStar>>(&expression))
        {
            return normalize_star(*star);
        }
        if (const auto* plus = std::get_if<std::shared_ptr<const Plus>>(&expression))
        {
            return normalize_plus(*plus);
        }
        if (const auto* concatenation =
                std::get_if<std::shared_ptr<const Concatenation>>(&expression))
        {
            return normalize_concatenation(*concatenation);
        }
        if (const auto* alternation = std::get_if<std::shared_ptr<const Alternation>>(&expression))
        {
            return normalize_alternation(*alternation);
        }
        if (const auto* intersection =
                std::get_if<std::shared_ptr<const Intersection>>(&expression))
        {
            return normalize_intersection(*intersection);
        }
        if (const auto* complement = std::get_if<std::shared_ptr<const Complement>>(&expression))
        {
            Expression inner = normalize(require_node(*complement).expression);

            if (is_empty(inner))
            {
                return universal();
            }

            if (is_universal(inner))
            {
                return EmptySet{};
            }

            if (const auto* nested = std::get_if<std::shared_ptr<const Complement>>(&inner))
            {
                return normalize(require_node(*nested).expression);
            }

            return make_complement(std::move(inner));
        }

        const auto& power = require_node(std::get<std::shared_ptr<const Power>>(expression));
        Expression inner = normalize(power.expression);
        if (power.exponent == 0)
        {
            return Epsilon{};
        }
        if (power.exponent == 1)
        {
            return inner;
        }
        if (is_empty(inner) || is_epsilon(inner))
        {
            return inner;
        }
        return make_power(std::move(inner), power.exponent);
    }
}
