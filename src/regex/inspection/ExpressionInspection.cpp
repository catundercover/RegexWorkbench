// Implements structural traversal and queries for regex trees.
#include "regex/inspection/ExpressionInspection.hpp"

#include <algorithm>
#include <memory>
#include <type_traits>
#include <variant>

namespace regex::inspection
{
    namespace
    {
        // Returns the raw target of a recursive AST pointer for null-aware comparison.
        template <typename Node>
        const Node* pointed(const std::shared_ptr<const Node>& node)
        {
            return node.get();
        }

        // Compares two ordered expression lists structurally.
        bool equal_list(const std::vector<Expression>& left, const std::vector<Expression>& right)
        {
            return left.size() == right.size() &&
                   std::equal(left.begin(), left.end(), right.begin(), structurally_equal);
        }

        // Invokes a visitor for each immediate child of an expression node.
        template <typename Visitor>
        void visit_children(const Expression& expression, Visitor&& visitor)
        {
            std::visit(
                [&visitor](const auto& node)
                {
                    using Node = std::decay_t<decltype(node)>;
                    if constexpr (
                        std::is_same_v<Node, std::shared_ptr<const KleeneStar>> ||
                        std::is_same_v<Node, std::shared_ptr<const Plus>> ||
                        std::is_same_v<Node, std::shared_ptr<const Complement>> ||
                        std::is_same_v<Node, std::shared_ptr<const Power>>
                    )
                    {
                        if (node)
                        {
                            visitor(node->expression);
                        }
                    }
                    else if constexpr (std::is_same_v<Node, std::shared_ptr<const Alternation>>)
                    {
                        if (node)
                        {
                            for (const Expression& child : node->alternatives)
                            {
                                visitor(child);
                            }
                        }
                    }
                    else if constexpr (std::is_same_v<Node, std::shared_ptr<const Intersection>>)
                    {
                        if (node)
                        {
                            for (const Expression& child : node->operands)
                            {
                                visitor(child);
                            }
                        }
                    }
                    else if constexpr (std::is_same_v<Node, std::shared_ptr<const Concatenation>>)
                    {
                        if (node)
                        {
                            for (const Expression& child : node->parts)
                            {
                                visitor(child);
                            }
                        }
                    }
                },
                expression
            );
        }

        // Recursively inserts terminal symbols into a sorted result set.
        void collect_terminals(const Expression& expression, std::set<char>& result)
        {
            if (const auto* terminal = std::get_if<Terminal>(&expression))
            {
                result.insert(terminal->value);
            }
            visit_children(
                expression, [&result](const Expression& child) { collect_terminals(child, result); }
            );
        }
    }

    bool structurally_equal(const Expression& left, const Expression& right)
    {
        if (left.index() != right.index())
        {
            return false;
        }
        if (const auto* terminal = std::get_if<Terminal>(&left))
        {
            return *terminal == std::get<Terminal>(right);
        }
        if (std::holds_alternative<Epsilon>(left) || std::holds_alternative<EmptySet>(left) ||
            std::holds_alternative<AnySymbol>(left))
        {
            return true;
        }
        if (const auto* node = std::get_if<std::shared_ptr<const KleeneStar>>(&left))
        {
            const auto& other = std::get<std::shared_ptr<const KleeneStar>>(right);
            return pointed(*node) && pointed(other)
                       ? structurally_equal((*node)->expression, other->expression)
                       : pointed(*node) == pointed(other);
        }
        if (const auto* node = std::get_if<std::shared_ptr<const Plus>>(&left))
        {
            const auto& other = std::get<std::shared_ptr<const Plus>>(right);
            return pointed(*node) && pointed(other)
                       ? structurally_equal((*node)->expression, other->expression)
                       : pointed(*node) == pointed(other);
        }
        if (const auto* node = std::get_if<std::shared_ptr<const Complement>>(&left))
        {
            const auto& other = std::get<std::shared_ptr<const Complement>>(right);
            return pointed(*node) && pointed(other)
                       ? structurally_equal((*node)->expression, other->expression)
                       : pointed(*node) == pointed(other);
        }
        if (const auto* node = std::get_if<std::shared_ptr<const Power>>(&left))
        {
            const auto& other = std::get<std::shared_ptr<const Power>>(right);
            return pointed(*node) && pointed(other)
                       ? (*node)->exponent == other->exponent &&
                             structurally_equal((*node)->expression, other->expression)
                       : pointed(*node) == pointed(other);
        }
        if (const auto* node = std::get_if<std::shared_ptr<const Alternation>>(&left))
        {
            const auto& other = std::get<std::shared_ptr<const Alternation>>(right);
            return pointed(*node) && pointed(other)
                       ? equal_list((*node)->alternatives, other->alternatives)
                       : pointed(*node) == pointed(other);
        }
        if (const auto* node = std::get_if<std::shared_ptr<const Intersection>>(&left))
        {
            const auto& other = std::get<std::shared_ptr<const Intersection>>(right);
            return pointed(*node) && pointed(other) ? equal_list((*node)->operands, other->operands)
                                                    : pointed(*node) == pointed(other);
        }
        const auto& node = std::get<std::shared_ptr<const Concatenation>>(left);
        const auto& other = std::get<std::shared_ptr<const Concatenation>>(right);
        return pointed(node) && pointed(other) ? equal_list(node->parts, other->parts)
                                               : pointed(node) == pointed(other);
    }

    bool contains_terminal(const Expression& expression)
    {
        if (std::holds_alternative<Terminal>(expression))
        {
            return true;
        }
        bool result = false;
        visit_children(
            expression,
            [&result](const Expression& child) { result = result || contains_terminal(child); }
        );
        return result;
    }

    bool contains_any_symbol(const Expression& expression)
    {
        if (std::holds_alternative<AnySymbol>(expression))
        {
            return true;
        }

        bool result = false;
        visit_children(
            expression,
            [&result](const Expression& child)
            { result = result || contains_any_symbol(child); }
        );
        return result;
    }

    std::set<char> terminals(const Expression& expression)
    {
        std::set<char> result;
        collect_terminals(expression, result);
        return result;
    }

    std::size_t node_count(const Expression& expression)
    {
        std::size_t result = 1;
        visit_children(
            expression, [&result](const Expression& child) { result += node_count(child); }
        );
        return result;
    }
}
