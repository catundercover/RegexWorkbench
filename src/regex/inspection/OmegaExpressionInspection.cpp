#include "regex/inspection/OmegaExpressionInspection.hpp"

#include "regex/inspection/ExpressionInspection.hpp"

#include <algorithm>
#include <memory>
#include <type_traits>
#include <variant>

namespace regex::inspection::omega
{
    namespace
    {
        template <typename Node>
        const Node* pointed(const std::shared_ptr<const Node>& node)
        {
            return node.get();
        }

        bool equal_list(
            const std::vector<regex::omega::Expression>& left,
            const std::vector<regex::omega::Expression>& right
        )
        {
            return left.size() == right.size() &&
                   std::equal(left.begin(), left.end(), right.begin(), structurally_equal);
        }

        template <typename Visitor>
        void visit_children(const regex::omega::Expression& expression, Visitor&& visitor)
        {
            std::visit(
                [&visitor](const auto& node)
                {
                    using Node = std::decay_t<decltype(node)>;

                    if constexpr (
                            std::is_same_v<Node, regex::omega::EmptySet> ||
                            std::is_same_v<Node, regex::omega::UniversalSet>
                        )
                    {
                        return;
                    }

                    if constexpr (
                        std::is_same_v<Node, std::shared_ptr<const regex::omega::Complement>>
                    )
                    {
                        if (node)
                        {
                            visitor(node->expression);
                        }
                    }
                    else if constexpr (
                        std::is_same_v<Node, std::shared_ptr<const regex::omega::Concatenation>>
                    )
                    {
                        if (node)
                        {
                            visitor(node->suffix);
                        }
                    }
                    else if constexpr (
                        std::is_same_v<Node, std::shared_ptr<const regex::omega::Alternation>>
                    )
                    {
                        if (node)
                        {
                            for (const regex::omega::Expression& child : node->alternatives)
                            {
                                visitor(child);
                            }
                        }
                    }
                    else if constexpr (
                        std::is_same_v<Node, std::shared_ptr<const regex::omega::Intersection>>
                    )
                    {
                        if (node)
                        {
                            for (const regex::omega::Expression& child : node->operands)
                            {
                                visitor(child);
                            }
                        }
                    }
                },
                expression
            );
        }
    }

    bool
    structurally_equal(const regex::omega::Expression& left, const regex::omega::Expression& right)
    {
        if (left.index() != right.index())
        {
            return false;
        }

        if (std::holds_alternative<regex::omega::EmptySet>(left))
        {
            return true;
        }

        if (std::holds_alternative<regex::omega::UniversalSet>(left))
        {
            return true;
        }

        if (const auto* left_power =
                std::get_if<std::shared_ptr<const regex::omega::OmegaPower>>(&left))
        {
            const auto& right_power =
                std::get<std::shared_ptr<const regex::omega::OmegaPower>>(right);

            return pointed(*left_power) && pointed(right_power)
                       ? regex::inspection::structurally_equal(
                             (*left_power)->expression, right_power->expression
                         )
                       : pointed(*left_power) == pointed(right_power);
        }

        if (const auto* left_concat =
                std::get_if<std::shared_ptr<const regex::omega::Concatenation>>(&left))
        {
            const auto& right_concat =
                std::get<std::shared_ptr<const regex::omega::Concatenation>>(right);

            return pointed(*left_concat) && pointed(right_concat)
                       ? regex::inspection::structurally_equal(
                             (*left_concat)->prefix, right_concat->prefix
                         ) && structurally_equal((*left_concat)->suffix, right_concat->suffix)
                       : pointed(*left_concat) == pointed(right_concat);
        }

        if (const auto* left_complement =
                std::get_if<std::shared_ptr<const regex::omega::Complement>>(&left))
        {
            const auto& right_complement =
                std::get<std::shared_ptr<const regex::omega::Complement>>(right);

            return pointed(*left_complement) && pointed(right_complement)
                       ? structurally_equal(
                             (*left_complement)->expression, right_complement->expression
                         )
                       : pointed(*left_complement) == pointed(right_complement);
        }

        if (const auto* left_alt =
                std::get_if<std::shared_ptr<const regex::omega::Alternation>>(&left))
        {
            const auto& right_alt =
                std::get<std::shared_ptr<const regex::omega::Alternation>>(right);

            return pointed(*left_alt) && pointed(right_alt)
                       ? equal_list((*left_alt)->alternatives, right_alt->alternatives)
                       : pointed(*left_alt) == pointed(right_alt);
        }

        const auto& left_intersection =
            std::get<std::shared_ptr<const regex::omega::Intersection>>(left);
        const auto& right_intersection =
            std::get<std::shared_ptr<const regex::omega::Intersection>>(right);

        return pointed(left_intersection) && pointed(right_intersection)
                   ? equal_list(left_intersection->operands, right_intersection->operands)
                   : pointed(left_intersection) == pointed(right_intersection);
    }

     std::set<char> terminals(const regex::omega::Expression& expression)
        {
            std::set<char> result;

            std::visit(
                [&result](const auto& node)
                {
                    using Node = std::decay_t<decltype(node)>;

                    if constexpr (
                            std::is_same_v<Node, regex::omega::EmptySet> ||
                            std::is_same_v<Node, regex::omega::UniversalSet>
                        )
                    {
                        return;
                    }
                    else if constexpr (
                        std::is_same_v<Node, std::shared_ptr<const regex::omega::OmegaPower>>
                    )
                    {
                        if (node)
                        {
                            const std::set<char> inner =
                                regex::inspection::terminals(node->expression);
                            result.insert(inner.begin(), inner.end());
                        }
                    }
                    else if constexpr (
                        std::is_same_v<Node, std::shared_ptr<const regex::omega::Concatenation>>
                    )
                    {
                        if (node)
                        {
                            const std::set<char> prefix =
                                regex::inspection::terminals(node->prefix);
                            const std::set<char> suffix = terminals(node->suffix);
                            result.insert(prefix.begin(), prefix.end());
                            result.insert(suffix.begin(), suffix.end());
                        }
                    }
                    else
                    {
                        visit_children(
                            regex::omega::Expression{node},
                            [&result](const regex::omega::Expression& child)
                            {
                                const std::set<char> child_terminals = terminals(child);
                                result.insert(child_terminals.begin(), child_terminals.end());
                            }
                        );
                    }
                },
                expression
            );

            return result;
        }

        bool contains_any_symbol(const regex::omega::Expression& expression)
        {
            bool result = false;

            std::visit(
                [&result](const auto& node)
                {
                    using Node = std::decay_t<decltype(node)>;

                    if constexpr (std::is_same_v<Node, regex::omega::EmptySet>)
                    {
                        return;
                    }
                    else if constexpr (std::is_same_v<Node, regex::omega::UniversalSet>)
                    {
                        result = true;
                    }
                    else if constexpr (
                        std::is_same_v<Node, std::shared_ptr<const regex::omega::OmegaPower>>
                    )
                    {
                        if (node)
                        {
                            result =
                                result || regex::inspection::contains_any_symbol(node->expression);
                        }
                    }
                    else if constexpr (
                        std::is_same_v<Node, std::shared_ptr<const regex::omega::Concatenation>>
                    )
                    {
                        if (node)
                        {
                            result =
                                result || regex::inspection::contains_any_symbol(node->prefix);
                            result = result || contains_any_symbol(node->suffix);
                        }
                    }
                    else
                    {
                        visit_children(
                            regex::omega::Expression{node},
                            [&result](const regex::omega::Expression& child)
                            { result = result || contains_any_symbol(child); }
                        );
                    }
                },
                expression
            );

            return result;
        }

    std::size_t node_count(const regex::omega::Expression& expression)
    {
        std::size_t result = 1;

        std::visit(
            [&result](const auto& node)
            {
                using Node = std::decay_t<decltype(node)>;

                if constexpr (
                        std::is_same_v<Node, regex::omega::EmptySet> ||
                        std::is_same_v<Node, regex::omega::UniversalSet>
                    )
                {
                    return;
                }
                else if constexpr (
                    std::is_same_v<Node, std::shared_ptr<const regex::omega::OmegaPower>>
                )
                    {
                    if (node)
                    {
                        result += regex::inspection::node_count(node->expression);
                    }
                }
                else if constexpr (
                    std::is_same_v<Node, std::shared_ptr<const regex::omega::Concatenation>>
                )
                {
                    if (node)
                    {
                        result += regex::inspection::node_count(node->prefix);
                        result += node_count(node->suffix);
                    }
                }
                else
                {
                    visit_children(
                        regex::omega::Expression{node},
                        [&result](const regex::omega::Expression& child)
                        { result += node_count(child); }
                    );
                }
            },
            expression
        );

        return result;
    }
}