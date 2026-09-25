// Implements validated construction of regular-expression AST nodes.
#include "regex/model/Expression.hpp"

#include <stdexcept>
#include <type_traits>
#include <utility>

namespace regex
{
    namespace
    {
        // Rejects an invalid null recursive node.
        template <typename Node>
        void require_node(const std::shared_ptr<const Node>& node)
        {
            if (!node)
            {
                throw std::invalid_argument("Regular-expression nodes cannot be null");
            }
        }

        // Validates the top-level pointer held by an expression variant.
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
    }

    Terminal::Terminal(const char terminal_value) : value(terminal_value)
    {
        if (!is_ascii_terminal(terminal_value))
        {
            throw std::invalid_argument("Terminals must be ASCII letters or digits");
        }
    }

    bool is_ascii_terminal(const char value)
    {
        return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
               (value >= '0' && value <= '9');
    }

    Expression make_terminal(const char value)
    {
        return Terminal{value};
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
            return make_complement(EmptySet{});
        }
        if (operands.size() == 1)
        {
            return std::move(operands.front());
        }
        return std::make_shared<const Intersection>(Intersection{std::move(operands)});
    }

    Expression make_concatenation(std::vector<Expression> parts)
    {
        for (const Expression& part : parts)
        {
            require_valid(part);
        }
        if (parts.empty())
        {
            return Epsilon{};
        }
        if (parts.size() == 1)
        {
            return std::move(parts.front());
        }
        return std::make_shared<const Concatenation>(Concatenation{std::move(parts)});
    }

    Expression make_star(Expression expression)
    {
        require_valid(expression);
        return std::make_shared<const KleeneStar>(KleeneStar{std::move(expression)});
    }

    Expression make_plus(Expression expression)
    {
        require_valid(expression);
        return std::make_shared<const Plus>(Plus{std::move(expression)});
    }

    Expression make_complement(Expression expression)
    {
        require_valid(expression);

        if (const auto* complement =
                std::get_if<std::shared_ptr<const Complement>>(&expression);
            complement != nullptr && *complement)
        {
            return (*complement)->expression;
        }

        return std::make_shared<const Complement>(Complement{std::move(expression)});
    }

    Expression make_power(Expression expression, const unsigned int exponent)
    {
        require_valid(expression);
        return std::make_shared<const Power>(Power{std::move(expression), exponent});
    }
}
