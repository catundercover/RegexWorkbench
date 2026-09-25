// Implements precedence-preserving omega regular-expression formatting.
#include "regex/formatting/OmegaRegexFormatter.hpp"

#include "regex/formatting/RegexFormatter.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

namespace regex::formatting::omega
{
    namespace
    {
        enum class Precedence : std::uint8_t
        {
            Alternation = 1,
            Intersection = 2,
            Concatenation = 3,
            Complement = 4,
            OmegaPower = 5,
            Atom = 6
        };

        template <typename Node>
        const Node& require_node(const std::shared_ptr<const Node>& node)
        {
            if (!node)
            {
                throw std::invalid_argument("Cannot format a null omega regular-expression node");
            }
            return *node;
        }

        Precedence precedence_of(const regex::omega::Expression& expression)
        {
            if (std::holds_alternative<std::shared_ptr<const regex::omega::Alternation>>(
                    expression
                ))
            {
                return Precedence::Alternation;
            }
            if (std::holds_alternative<std::shared_ptr<const regex::omega::Intersection>>(
                    expression
                ))
            {
                return Precedence::Intersection;
            }
            if (std::holds_alternative<std::shared_ptr<const regex::omega::Concatenation>>(
                    expression
                ))
            {
                return Precedence::Concatenation;
            }
            if (std::holds_alternative<std::shared_ptr<const regex::omega::Complement>>(expression))
            {
                return Precedence::Complement;
            }
            if (std::holds_alternative<std::shared_ptr<const regex::omega::OmegaPower>>(expression))
            {
                return Precedence::OmegaPower;
            }
            return Precedence::Atom;
        }

        bool finite_operand_needs_parentheses(const regex::Expression& expression)
        {
            return std::holds_alternative<std::shared_ptr<const regex::Alternation>>(expression) ||
                   std::holds_alternative<std::shared_ptr<const regex::Intersection>>(expression) ||
                   std::holds_alternative<std::shared_ptr<const regex::Concatenation>>(
                       expression
                   ) ||
                   std::holds_alternative<std::shared_ptr<const regex::Complement>>(expression);
        }

        std::string render_omega_power_operand(const regex::Expression& expression)
        {
            std::string result = regex::formatting::format(expression);
            if (finite_operand_needs_parentheses(expression))
            {
                result = "(" + result + ")";
            }
            return result;
        }

        std::string render_finite_prefix(const regex::Expression& expression)
        {
            std::string result = regex::formatting::format(expression);
            if (std::holds_alternative<std::shared_ptr<const regex::Alternation>>(expression) ||
                std::holds_alternative<std::shared_ptr<const regex::Intersection>>(expression))
            {
                result = "(" + result + ")";
            }
            return result;
        }

        std::string render(const regex::omega::Expression& expression);

        std::string render_child(const regex::omega::Expression& child, const Precedence parent)
        {
            std::string result = render(child);
            if (static_cast<int>(precedence_of(child)) <= static_cast<int>(parent))
            {
                result = "(" + result + ")";
            }
            return result;
        }

        std::string join(
            const std::vector<regex::omega::Expression>& expressions,
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

        std::string render(const regex::omega::Expression& expression)
        {
            return std::visit(
                [](const auto& node) -> std::string
                {
                    using Node = std::decay_t<decltype(node)>;

                    if constexpr (std::is_same_v<Node, regex::omega::EmptySet>)
                    {
                        return "\xE2\x88\x85";
                    }
                    else if constexpr (std::is_same_v<Node, regex::omega::UniversalSet>)
                    {
                        return "\xCE\xA3^ω";
                    }
                    else if constexpr (
                        std::is_same_v<Node, std::shared_ptr<const regex::omega::OmegaPower>>
                    )
                    {
                        return render_omega_power_operand(require_node(node).expression) + "^ω";
                    }
                    else if constexpr (
                        std::is_same_v<Node, std::shared_ptr<const regex::omega::Concatenation>>
                    )
                    {
                        const auto& concat = require_node(node);
                        return render_finite_prefix(concat.prefix) +
                               render_child(concat.suffix, Precedence::Concatenation);
                    }
                    else if constexpr (
                        std::is_same_v<Node, std::shared_ptr<const regex::omega::Complement>>
                    )
                    {
                        return "!" +
                               render_child(require_node(node).expression, Precedence::Complement);
                    }
                    else if constexpr (
                        std::is_same_v<Node, std::shared_ptr<const regex::omega::Intersection>>
                    )
                    {
                        const auto& operands = require_node(node).operands;
                        return operands.empty() ? "\xCE\xA3^ω"
                                                : join(operands, "&", Precedence::Intersection);
                    }
                    else
                    {
                        const auto& alternatives = require_node(node).alternatives;
                        return alternatives.empty()
                                   ? "\xE2\x88\x85"
                                   : join(alternatives, "|", Precedence::Alternation);
                    }
                },
                expression
            );
        }
    }

    std::string format(const regex::omega::Expression& expression)
    {
        return render(expression);
    }
}
