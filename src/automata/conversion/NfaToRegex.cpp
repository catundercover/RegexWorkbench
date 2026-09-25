// Implements automaton-to-regex conversion by state elimination.
#include "automata/conversion/NfaToRegex.hpp"

#include "automata/conversion/MataAdapter.hpp"
#include "regex/formatting/RegexFormatter.hpp"

#include <memory>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>
#include <optional>
#include <limits>
#include <unordered_map>

namespace automata::conversion
{
    namespace
    {
        // Internal expression representing the empty language.
        struct Empty
        {
        };
        // Internal expression representing the empty word.
        struct Epsilon
        {
        };
        // Internal expression representing one terminal symbol.
        struct Literal
        {
            Symbol symbol;
        };
        // Forward declaration for recursive internal expression nodes.
        struct Expression;
        // Immutable shared pointer used by the state-elimination matrix.
        using ExpressionPtr = std::shared_ptr<const Expression>;
        // Internal binary union expression.
        struct Union
        {
            ExpressionPtr left;
            ExpressionPtr right;
        };
        // Internal binary concatenation expression.
        struct Concatenation
        {
            ExpressionPtr left;
            ExpressionPtr right;
        };
        // Internal Kleene-star expression.
        struct Star
        {
            ExpressionPtr inner;
        };

        // Compact expression representation used during state elimination.
        struct Expression
        {
            std::variant<Empty, Epsilon, Literal, Union, Concatenation, Star> value;
        };

        using ExpressionSizeCache = std::unordered_map<ExpressionPtr, std::size_t>;

        // Constructs an internal empty-language expression.
        ExpressionPtr empty()
        {
            return std::make_shared<Expression>(Expression{Empty{}});
        }

        // Constructs an internal empty-word expression.
        ExpressionPtr epsilon()
        {
            return std::make_shared<Expression>(Expression{Epsilon{}});
        }

        // Constructs an internal terminal expression.
        ExpressionPtr literal(Symbol symbol)
        {
            return std::make_shared<Expression>(Expression{Literal{symbol}});
        }

        // Returns whether an internal expression is the empty language.
        bool is_empty(const ExpressionPtr& expression)
        {
            return std::holds_alternative<Empty>(expression->value);
        }

        // Returns whether an internal expression is the empty word.
        bool is_epsilon(const ExpressionPtr& expression)
        {
            return std::holds_alternative<Epsilon>(expression->value);
        }

        // Compares internal expressions while treating union as commutative.
        bool equal(const ExpressionPtr& left, const ExpressionPtr& right)
        {
            if (left->value.index() != right->value.index())
                return false;
            if (is_empty(left) || is_epsilon(left))
                return true;

            if (const auto* left_literal = std::get_if<Literal>(&left->value))
            {
                return left_literal->symbol == std::get<Literal>(right->value).symbol;
            }
            if (const auto* left_union = std::get_if<Union>(&left->value))
            {
                const auto& right_union = std::get<Union>(right->value);
                return (equal(left_union->left, right_union.left) &&
                        equal(left_union->right, right_union.right)) ||
                       (equal(left_union->left, right_union.right) &&
                        equal(left_union->right, right_union.left));
            }
            if (const auto* left_concat = std::get_if<Concatenation>(&left->value))
            {
                const auto& right_concat = std::get<Concatenation>(right->value);
                return equal(left_concat->left, right_concat.left) &&
                       equal(left_concat->right, right_concat.right);
            }

            const auto& left_star = std::get<Star>(left->value);
            return equal(left_star.inner, std::get<Star>(right->value).inner);
        }

        std::size_t expression_size(
                 const ExpressionPtr& expression, ExpressionSizeCache& size_cache
             )
        {
            if (const auto cached = size_cache.find(expression); cached != size_cache.end())
            {
                return cached->second;
            }

            std::size_t size = 1;
            if (const auto* union_item = std::get_if<Union>(&expression->value))
            {
                size += expression_size(union_item->left, size_cache);
                size += expression_size(union_item->right, size_cache);
            }
            else if (const auto* concatenation_item =
                         std::get_if<Concatenation>(&expression->value))
            {
                size += expression_size(concatenation_item->left, size_cache);
                size += expression_size(concatenation_item->right, size_cache);
            }
            else if (const auto* star_item = std::get_if<Star>(&expression->value))
            {
                size += expression_size(star_item->inner, size_cache);
            }

            size_cache.emplace(expression, size);
            return size;
        }

        std::size_t elimination_cost(
                 const std::vector<std::vector<ExpressionPtr>>& labels,
                 StateId eliminated,
                 const std::vector<bool>& remaining,
                 ExpressionSizeCache& size_cache
             )
        {
            std::size_t incoming = 0;
            std::size_t outgoing = 0;
            std::size_t size_sum = 1;

            for (StateId state = 0; state < labels.size(); ++state)
            {
                if (!remaining[state] || state == eliminated)
                {
                    continue;
                }

                if (!is_empty(labels[state][eliminated]))
                {
                    ++incoming;
                    size_sum += expression_size(labels[state][eliminated], size_cache);
                }

                if (!is_empty(labels[eliminated][state]))
                {
                    ++outgoing;
                    size_sum += expression_size(labels[eliminated][state], size_cache);
                }
            }

            if (!is_empty(labels[eliminated][eliminated]))
            {
                size_sum += expression_size(labels[eliminated][eliminated], size_cache);
            }

            if (incoming == 0 || outgoing == 0)
            {
                return 0;
            }

            constexpr std::size_t Max = std::numeric_limits<std::size_t>::max();
            if (incoming > Max / outgoing)
            {
                return Max;
            }

            const std::size_t degree_product = incoming * outgoing;
            if (size_sum > Max / degree_product)
            {
                return Max;
            }

            return degree_product * size_sum;
        }

        // Constructs a union while applying empty and idempotence identities.
        ExpressionPtr unite(const ExpressionPtr& left, const ExpressionPtr& right)
        {
            if (is_empty(left))
                return right;
            if (is_empty(right) || equal(left, right))
                return left;
            return std::make_shared<Expression>(Expression{Union{left, right}});
        }

        // Constructs concatenation while applying empty and epsilon identities.
        ExpressionPtr concatenate(const ExpressionPtr& left, const ExpressionPtr& right)
        {
            if (is_empty(left) || is_empty(right))
                return empty();
            if (is_epsilon(left))
                return right;
            if (is_epsilon(right))
                return left;
            return std::make_shared<Expression>(Expression{Concatenation{left, right}});
        }

        // Constructs a star while collapsing empty, epsilon, and nested stars.
        ExpressionPtr star(const ExpressionPtr& inner)
        {
            if (is_empty(inner) || is_epsilon(inner))
                return epsilon();
            if (std::holds_alternative<Star>(inner->value))
                return inner;
            return std::make_shared<Expression>(Expression{Star{inner}});
        }

        // Converts one automaton transition label to an internal expression.
        ExpressionPtr transition_expression(const Transition& transition)
        {
            return transition.symbol ? literal(*transition.symbol) : epsilon();
        }

        void
       eliminate_one_state(std::vector<std::vector<ExpressionPtr>>& labels, StateId eliminated)
        {
            const ExpressionPtr loop = star(labels[eliminated][eliminated]);
            for (StateId source = 0; source < labels.size(); ++source)
            {
                if (source == eliminated || is_empty(labels[source][eliminated]))
                {
                    continue;
                }

                for (StateId target = 0; target < labels.size(); ++target)
                {
                    if (target == eliminated || is_empty(labels[eliminated][target]))
                    {
                        continue;
                    }

                    const ExpressionPtr path = concatenate(
                        concatenate(labels[source][eliminated], loop), labels[eliminated][target]
                    );
                    labels[source][target] = unite(labels[source][target], path);
                }
            }

            for (StateId state = 0; state < labels.size(); ++state)
            {
                labels[state][eliminated] = empty();
                labels[eliminated][state] = empty();
            }
        }

        // Builds a generalized automaton and eliminates each original state.
        ExpressionPtr eliminate_states(const Nfa& input)
        {
            // Minimization reduces the cubic state-elimination matrix and resulting expression.
            const Nfa dfa = detail::from_mata(detail::minimize(detail::to_mata(input)));
            const std::size_t state_count = dfa.transitions.size();
            const StateId synthetic_start = state_count;
            const StateId synthetic_final = state_count + 1;
            const std::size_t total_state_count = state_count + 2;

            std::vector labels(
                total_state_count, std::vector<ExpressionPtr>(total_state_count, empty())
            );

            for (StateId source = 0; source < state_count; ++source)
            {
                for (const Transition& transition : dfa.transitions[source])
                {
                    labels[source][transition.target] =
                        unite(labels[source][transition.target], transition_expression(transition));
                }
            }

            labels[synthetic_start][dfa.start] = epsilon();
            for (StateId final_state : dfa.finals)
            {
                labels[final_state][synthetic_final] =
                    unite(labels[final_state][synthetic_final], epsilon());
            }

            std::vector<bool> remaining(total_state_count, true);
            remaining[synthetic_start] = false;
            remaining[synthetic_final] = false;

            ExpressionSizeCache size_cache;

            for (std::size_t step = 0; step < state_count; ++step)
            {
                std::optional<StateId> best_state;
                std::size_t best_cost = 0;

                for (StateId state = 0; state < state_count; ++state)
                {
                    if (!remaining[state])
                    {
                        continue;
                    }

                    const std::size_t cost =
                        elimination_cost(labels, state, remaining, size_cache);
                    if (!best_state.has_value() || cost < best_cost)
                    {
                        best_state = state;
                        best_cost = cost;
                    }
                }

                if (!best_state.has_value())
                {
                    break;
                }

                eliminate_one_state(labels, *best_state);
                remaining[*best_state] = false;
            }

            return labels[synthetic_start][synthetic_final];
        }

        // Converts the internal binary representation to the public regex AST.
        regex::Expression to_ast(const ExpressionPtr& expression)
        {
            if (is_empty(expression))
                return regex::EmptySet{};
            if (is_epsilon(expression))
                return regex::Epsilon{};
            if (const auto* item = std::get_if<Literal>(&expression->value))
            {
                return regex::make_terminal(item->symbol);
            }
            if (const auto* item = std::get_if<Union>(&expression->value))
            {
                return regex::make_alternation({to_ast(item->left), to_ast(item->right)});
            }
            if (const auto* item = std::get_if<Concatenation>(&expression->value))
            {
                return regex::make_concatenation({to_ast(item->left), to_ast(item->right)});
            }
            if (const auto* item = std::get_if<Star>(&expression->value))
            {
                return regex::make_star(to_ast(item->inner));
            }
            throw std::logic_error("Unknown state-elimination expression");
        }
    }

    regex::Expression to_regex_ast(const Nfa& nfa)
    {
        return to_ast(eliminate_states(nfa));
    }

    std::string to_regex(const Nfa& nfa)
    {
        return regex::formatting::format(to_regex_ast(nfa));
    }
}
