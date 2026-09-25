// Implements semantics-preserving omega expression normalization.
#include "regex/simplification/OmegaRegexSimplifier.hpp"

#include "regex/inspection/OmegaExpressionInspection.hpp"
#include "regex/simplification/RegexSimplifier.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

namespace regex::simplification::omega
{
    namespace
    {
        bool is_empty(const regex::omega::Expression& expression)
        {
            return std::holds_alternative<regex::omega::EmptySet>(expression);
        }

        bool is_universal(const regex::omega::Expression& expression)
        {
            return std::holds_alternative<regex::omega::UniversalSet>(expression);
        }

        bool finite_is_empty(const regex::Expression& expression)
        {
            return std::holds_alternative<regex::EmptySet>(expression);
        }

        bool finite_is_epsilon(const regex::Expression& expression)
        {
            return std::holds_alternative<regex::Epsilon>(expression);
        }

        template <typename Node>
        const Node& require_node(const std::shared_ptr<const Node>& node)
        {
            if (!node)
            {
                throw std::invalid_argument(
                    "Cannot normalize a null omega regular-expression node"
                );
            }
            return *node;
        }

        void add_unique(
            std::vector<regex::omega::Expression>& expressions, regex::omega::Expression candidate
        )
        {
            const bool duplicate = std::any_of(
                expressions.begin(),
                expressions.end(),
                [&candidate](const regex::omega::Expression& existing)
                { return regex::inspection::omega::structurally_equal(existing, candidate); }
            );

            if (!duplicate)
            {
                expressions.push_back(std::move(candidate));
            }
        }

        bool is_complement_of(
                const regex::omega::Expression& expression,
                const regex::omega::Expression& possible_inner
            )
        {
            const auto* complement =
                std::get_if<std::shared_ptr<const regex::omega::Complement>>(&expression);

            return complement && *complement &&
                   regex::inspection::omega::structurally_equal(
                       (*complement)->expression, possible_inner
                   );
        }

        bool contains_complement_pair(
            const std::vector<regex::omega::Expression>& expressions,
            const regex::omega::Expression& candidate
        )
        {
            return std::any_of(
                expressions.begin(),
                expressions.end(),
                [&candidate](const regex::omega::Expression& existing)
                {
                    return is_complement_of(existing, candidate) ||
                           is_complement_of(candidate, existing);
                }
            );
        }

        regex::omega::Expression
            normalize_omega_power(const std::shared_ptr<const regex::omega::OmegaPower>& node)
        {
            regex::Expression inner =
                regex::simplification::normalize(require_node(node).expression);

            if (finite_is_empty(inner) || finite_is_epsilon(inner))
            {
                return regex::omega::EmptySet{};
            }

            if (std::holds_alternative<regex::AnySymbol>(inner))
            {
                return regex::omega::UniversalSet{};
            }

            return regex::omega::make_omega_power(std::move(inner));
        }

        regex::omega::Expression
        normalize_concatenation(const std::shared_ptr<const regex::omega::Concatenation>& node)
        {
            regex::Expression prefix = regex::simplification::normalize(require_node(node).prefix);
            regex::omega::Expression suffix = normalize(require_node(node).suffix);

            if (finite_is_empty(prefix) || is_empty(suffix))
            {
                return regex::omega::EmptySet{};
            }

            if (finite_is_epsilon(prefix))
            {
                return suffix;
            }

            return regex::omega::make_concatenation(std::move(prefix), std::move(suffix));
        }

        regex::omega::Expression
            normalize_alternation(const std::shared_ptr<const regex::omega::Alternation>& node)
        {
            std::vector<regex::omega::Expression> alternatives;

            for (const regex::omega::Expression& alternative : require_node(node).alternatives)
            {
                regex::omega::Expression normalized = normalize(alternative);

                if (is_universal(normalized))
                {
                    return regex::omega::UniversalSet{};
                }

                if (is_empty(normalized))
                {
                    continue;
                }

                if (contains_complement_pair(alternatives, normalized))
                {
                    return regex::omega::UniversalSet{};
                }

                if (const auto* nested =
                        std::get_if<std::shared_ptr<const regex::omega::Alternation>>(&normalized))
                {
                    for (const regex::omega::Expression& child : require_node(*nested).alternatives)
                    {
                        if (is_universal(child) || contains_complement_pair(alternatives, child))
                        {
                            return regex::omega::UniversalSet{};
                        }
                        add_unique(alternatives, child);
                    }
                }
                else
                {
                    add_unique(alternatives, std::move(normalized));
                }
            }

            return regex::omega::make_alternation(std::move(alternatives));
        }

        regex::omega::Expression
            normalize_intersection(const std::shared_ptr<const regex::omega::Intersection>& node)
        {
            std::vector<regex::omega::Expression> operands;

            for (const regex::omega::Expression& operand : require_node(node).operands)
            {
                regex::omega::Expression normalized = normalize(operand);

                if (is_empty(normalized))
                {
                    return regex::omega::EmptySet{};
                }

                if (is_universal(normalized))
                {
                    continue;
                }

                if (contains_complement_pair(operands, normalized))
                {
                    return regex::omega::EmptySet{};
                }

                if (const auto* nested =
                        std::get_if<std::shared_ptr<const regex::omega::Intersection>>(&normalized))
                {
                    for (const regex::omega::Expression& child : require_node(*nested).operands)
                    {
                        if (is_empty(child) || contains_complement_pair(operands, child))
                        {
                            return regex::omega::EmptySet{};
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

            return regex::omega::make_intersection(std::move(operands));
        }
    }

    regex::omega::Expression normalize(const regex::omega::Expression& expression)
    {
        if (std::holds_alternative<regex::omega::EmptySet>(expression))
        {
            return expression;
        }

        if (std::holds_alternative<regex::omega::UniversalSet>(expression))
        {
            return expression;
        }

        if (const auto* power =
                std::get_if<std::shared_ptr<const regex::omega::OmegaPower>>(&expression))
        {
            return normalize_omega_power(*power);
        }

        if (const auto* concatenation =
                std::get_if<std::shared_ptr<const regex::omega::Concatenation>>(&expression))
        {
            return normalize_concatenation(*concatenation);
        }

        if (const auto* alternation =
                std::get_if<std::shared_ptr<const regex::omega::Alternation>>(&expression))
        {
            return normalize_alternation(*alternation);
        }

        if (const auto* intersection =
                std::get_if<std::shared_ptr<const regex::omega::Intersection>>(&expression))
        {
            return normalize_intersection(*intersection);
        }

        const auto& complement =
            require_node(std::get<std::shared_ptr<const regex::omega::Complement>>(expression));

        regex::omega::Expression inner = normalize(complement.expression);

        if (is_empty(inner))
        {
            return regex::omega::UniversalSet{};
        }

        if (is_universal(inner))
        {
            return regex::omega::EmptySet{};
        }

        if (const auto* nested =
                std::get_if<std::shared_ptr<const regex::omega::Complement>>(&inner))
        {
            return normalize(require_node(*nested).expression);
        }

        return regex::omega::make_complement(std::move(inner));
    }
}
