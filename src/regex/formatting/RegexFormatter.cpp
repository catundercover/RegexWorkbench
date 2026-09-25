// Implements precedence-aware formatting of regex expression trees.
#include "regex/formatting/RegexFormatter.hpp"

#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <variant>

namespace regex::formatting
{
    namespace
    {
        // UTF-8 spelling of the any-symbol literal.
        constexpr std::string_view Sigma = "\xCE\xA3";

        // Parser precedence levels ordered from weakest to strongest.
        enum class Precedence : std::uint8_t
        {
            Alternation = 1,
            Intersection = 2,
            Concatenation = 3,
            Complement = 4,
            Repetition = 5,
            Power = 6,
            Atom = 7
        };

        // Returns the precedence of an expression's root node.
        Precedence precedence_of(const Expression& expression)
        {
            if (std::holds_alternative<std::shared_ptr<const Alternation>>(expression))
            {
                return Precedence::Alternation;
            }
            if (std::holds_alternative<std::shared_ptr<const Intersection>>(expression))
            {
                return Precedence::Intersection;
            }
            if (std::holds_alternative<std::shared_ptr<const Concatenation>>(expression))
            {
                return Precedence::Concatenation;
            }
            if (std::holds_alternative<std::shared_ptr<const Complement>>(expression))
            {
                return Precedence::Complement;
            }
            if (std::holds_alternative<std::shared_ptr<const KleeneStar>>(expression) ||
                std::holds_alternative<std::shared_ptr<const Plus>>(expression))
            {
                return Precedence::Repetition;
            }
            if (std::holds_alternative<std::shared_ptr<const Power>>(expression))
            {
                return Precedence::Power;
            }
            return Precedence::Atom;
        }

        // Formats one complete expression subtree.
        std::string render(const Expression& expression);

        // Formats a child and adds parentheses when precedence would change its meaning.
        std::string render_child(const Expression& child, const Precedence parent)
        {
            std::string result = render(child);
            if (static_cast<int>(precedence_of(child)) <= static_cast<int>(parent))
            {
                result = "(" + result + ")";
            }
            return result;
        }

        // Dereferences a recursive AST node or rejects an invalid null pointer.
        template <typename Node>
        const Node& require_node(const std::shared_ptr<const Node>& node)
        {
            if (!node)
            {
                throw std::invalid_argument("Cannot format a null regular-expression node");
            }
            return *node;
        }

        // Formats and joins same-precedence operands with a separator.
        std::string join(
            const std::vector<Expression>& expressions,
            const std::string_view separator,
            const Precedence precedence
        )
        {
            std::string result;
            for (std::size_t index = 0; index < expressions.size(); ++index)
            {
                if (index != 0)
                {
                    result += separator;
                }
                result += render_child(expressions[index], precedence);
            }
            return result;
        }

        // Formats one expression node using canonical operator spellings.
        std::string render(const Expression& expression)
        {
            return std::visit(
                [](const auto& node) -> std::string
                {
                    using Node = std::decay_t<decltype(node)>;
                    if constexpr (std::is_same_v<Node, Terminal>)
                    {
                        return std::string(1, node.value);
                    }
                    else if constexpr (std::is_same_v<Node, Epsilon>)
                    {
                        return "\xCE\xB5";
                    }
                    else if constexpr (std::is_same_v<Node, EmptySet>)
                    {
                        return "\xE2\x88\x85";
                    }
                    else if constexpr (std::is_same_v<Node, AnySymbol>)
                    {
                        return std::string(Sigma);
                    }
                    else if constexpr (std::is_same_v<Node, std::shared_ptr<const KleeneStar>>)
                    {
                        return render_child(require_node(node).expression, Precedence::Repetition) +
                               "*";
                    }
                    else if constexpr (std::is_same_v<Node, std::shared_ptr<const Plus>>)
                    {
                        return render_child(require_node(node).expression, Precedence::Repetition) +
                               "+";
                    }
                    else if constexpr (std::is_same_v<Node, std::shared_ptr<const Complement>>)
                    {
                        return "!" +
                               render_child(require_node(node).expression, Precedence::Complement);
                    }
                    else if constexpr (std::is_same_v<Node, std::shared_ptr<const Concatenation>>)
                    {
                        const auto& parts = require_node(node).parts;
                        return parts.empty() ? "\xCE\xB5"
                                             : join(parts, "", Precedence::Concatenation);
                    }
                    else if constexpr (std::is_same_v<Node, std::shared_ptr<const Intersection>>)
                    {
                        const auto& operands = require_node(node).operands;
                        return operands.empty() ? "!\xE2\x88\x85"
                                                : join(operands, "&", Precedence::Intersection);
                    }
                    else if constexpr (std::is_same_v<Node, std::shared_ptr<const Alternation>>)
                    {
                        const auto& alternatives = require_node(node).alternatives;
                        return alternatives.empty()
                                   ? "\xE2\x88\x85"
                                   : join(alternatives, "|", Precedence::Alternation);
                    }
                    else
                    {
                        const Power& power = require_node(node);
                        return render_child(power.expression, Precedence::Power) + "^" +
                               std::to_string(power.exponent);
                    }
                },
                expression
            );
        }
    }

    std::string format(const Expression& expression)
    {
        return render(expression);
    }
}
