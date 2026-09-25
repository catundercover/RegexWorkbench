// Implements validated construction of omega-regular-expression AST nodes.
#include "regex/model/OmegaExpression.hpp"

#include <stdexcept>
#include <type_traits>
#include <utility>

namespace regex::omega
{
    namespace
    {
        template <typename Node>
        void require_node(const std::shared_ptr<const Node>& node)
        {
            if (!node)
            {
                throw std::invalid_argument("Omega regular-expression nodes cannot be null");
            }
        }

        void require_valid(const Expression& expression)
        {
            std::visit(
                [](const auto& node)
                {
                    using Node = std::decay_t<decltype(node)>;
                    if constexpr (requires { typename Node::element_type; })
                    {
                        require_node(node);
                    }
                },
                expression
            );
        }

        void require_valid_finite(const regex::Expression& expression)
        {
            std::visit(
                [](const auto& node)
                {
                    using Node = std::decay_t<decltype(node)>;
                    if constexpr (requires { typename Node::element_type; })
                    {
                        require_node(node);
                    }
                },
                expression
            );
        }
    }

    Expression make_alternation(std::vector<Expression> alternatives)
    {
        for (const Expression& alternative : alternatives)
        {
            require_valid(alternative);
        }
        if (alternatives.empty())
        {
            return EmptySet{};
        }
        if (alternatives.size() == 1)
        {
            return std::move(alternatives.front());
        }
        return std::make_shared<const Alternation>(Alternation{std::move(alternatives)});
    }

    Expression make_intersection(std::vector<Expression> operands)
    {
        for (const Expression& operand : operands)
        {
            require_valid(operand);
        }
        if (operands.empty())
        {
            return UniversalSet{};
        }
        if (operands.size() == 1)
        {
            return std::move(operands.front());
        }
        return std::make_shared<const Intersection>(Intersection{std::move(operands)});
    }

    Expression make_concatenation(regex::Expression prefix, Expression suffix)
    {
        require_valid_finite(prefix);
        require_valid(suffix);
        return std::make_shared<const Concatenation>(
            Concatenation{std::move(prefix), std::move(suffix)}
        );
    }

    Expression make_complement(Expression expression)
    {
        require_valid(expression);

        if (const auto* complement =
                std::get_if<std::shared_ptr<const Complement>>(&expression))
        {
            return (*complement)->expression;
        }

        return std::make_shared<const Complement>(Complement{std::move(expression)});
    }

    Expression make_omega_power(regex::Expression expression)
    {
        require_valid_finite(expression);
        return std::make_shared<const OmegaPower>(OmegaPower{std::move(expression)});
    }
}
