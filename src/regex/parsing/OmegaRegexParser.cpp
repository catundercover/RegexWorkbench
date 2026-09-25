// Implements normalized parsing for omega regular expressions.
#include "regex/parsing/OmegaRegexParser.hpp"

#include "regex/parsing/InputNormalization.hpp"
#include "regex/parsing/RegexParser.hpp"

#include <cstddef>
#include <string_view>
#include <utility>

namespace regex::parsing::omega
{
    namespace
    {
        struct FiniteMatch
        {
            std::size_t end = 0;
            regex::Expression expression;
        };

        class Parser
        {
        public:
            explicit Parser(const std::string_view input) : input_(input)
            {
            }

            [[nodiscard]] std::optional<regex::omega::Expression> parse()
            {
                std::optional<regex::omega::Expression> expression = parse_alternation();
                if (!expression.has_value())
                {
                    return std::nullopt;
                }

                if (position_ != input_.size())
                {
                    record_error(position_, trailing_input_message(position_));
                    return std::nullopt;
                }

                return expression;
            }

            [[nodiscard]] std::size_t error_offset() const
            {
                return error_offset_;
            }

            [[nodiscard]] const std::string& error_message() const
            {
                return error_message_;
            }

        private:
            [[nodiscard]] bool starts_with(
                const std::size_t position,
                const std::string_view token
            ) const
            {
                return input_.substr(position).starts_with(token);
            }

            [[nodiscard]] bool starts_finite_expression(const std::size_t position) const
            {
                return position < input_.size() &&
                       (regex::is_ascii_terminal(input_[position]) || input_[position] == '(' ||
                        input_[position] == '!' || input_[position] == '~' ||
                        starts_with(position, "ε") || starts_with(position, "∅") ||
                        starts_with(position, "Σ"));
            }

            [[nodiscard]] bool cannot_continue_finite_prefix(const std::size_t position) const
            {
                if (position >= input_.size())
                {
                    return true;
                }

                if (starts_with(position, "^ω"))
                {
                    return true;
                }

                return input_[position] == '|' || input_[position] == '&' || input_[position] == ')';
            }

            [[nodiscard]] std::string trailing_input_message(const std::size_t position) const
            {
                if (starts_finite_expression(position))
                {
                    return "cannot concatenate another expression after an omega expression";
                }

                return "expected the end of the omega regular expression";
            }

            bool consume(const std::string_view token)
            {
                if (!starts_with(position_, token))
                {
                    return false;
                }

                position_ += token.size();
                return true;
            }

            void record_error(const std::size_t offset, std::string message)
            {
                if (offset > error_offset_ ||
                    error_message_ == "invalid omega regular-expression syntax")
                {
                    error_offset_ = offset;
                    error_message_ = std::move(message);
                }
            }

            [[nodiscard]] std::optional<std::size_t>
            scan_balanced_parenthesized(const std::size_t begin) const
            {
                if (begin >= input_.size() || input_[begin] != '(')
                {
                    return std::nullopt;
                }

                std::size_t depth = 1;
                for (std::size_t cursor = begin + 1; cursor < input_.size(); ++cursor)
                {
                    if (input_[cursor] == '(')
                    {
                        ++depth;
                    }
                    else if (input_[cursor] == ')' && --depth == 0)
                    {
                        return cursor + 1;
                    }
                }

                return std::nullopt;
            }

            [[nodiscard]] std::optional<std::size_t>
            scan_finite_atom(const std::size_t begin) const
            {
                if (begin >= input_.size())
                {
                    return std::nullopt;
                }

                if (regex::is_ascii_terminal(input_[begin]))
                {
                    return begin + 1;
                }

                for (const std::string_view literal :
                     {std::string_view{"ε"}, std::string_view{"∅"}, std::string_view{"Σ"}})
                {
                    if (starts_with(begin, literal))
                    {
                        return begin + literal.size();
                    }
                }

                return scan_balanced_parenthesized(begin);
            }

            [[nodiscard]] std::optional<FiniteMatch>
            finite_repetition_at(const std::size_t begin) const
            {
                std::optional<std::size_t> end = scan_finite_atom(begin);
                if (!end.has_value())
                {
                    return std::nullopt;
                }

                if (*end < input_.size() && input_[*end] == '^' && !starts_with(*end, "^ω"))
                {
                    std::size_t cursor = *end + 1;
                    const std::size_t digits_begin = cursor;

                    while (cursor < input_.size() && input_[cursor] >= '0' && input_[cursor] <= '9')
                    {
                        ++cursor;
                    }

                    if (cursor == digits_begin)
                    {
                        return std::nullopt;
                    }

                    end = cursor;
                }

                if (*end < input_.size() && (input_[*end] == '*' || input_[*end] == '+'))
                {
                    ++*end;
                }

                regex::parsing::ParseResult parsed =
                    regex::parsing::parse(input_.substr(begin, *end - begin));

                if (!parsed.expression.has_value() || parsed.error.has_value())
                {
                    return std::nullopt;
                }

                return FiniteMatch{*end, std::move(*parsed.expression)};
            }

            [[nodiscard]] std::optional<FiniteMatch>
            finite_complement_at(const std::size_t begin) const
            {
                std::size_t operand_begin = begin;
                bool has_complement = false;

                if (operand_begin < input_.size() &&
                    (input_[operand_begin] == '!' || input_[operand_begin] == '~'))
                {
                    ++operand_begin;
                    has_complement = true;
                }

                std::optional<FiniteMatch> operand = finite_repetition_at(operand_begin);
                if (!operand.has_value())
                {
                    return std::nullopt;
                }

                if (!has_complement)
                {
                    return operand;
                }

                regex::parsing::ParseResult parsed =
                    regex::parsing::parse(input_.substr(begin, operand->end - begin));

                if (!parsed.expression.has_value() || parsed.error.has_value())
                {
                    return std::nullopt;
                }

                return FiniteMatch{operand->end, std::move(*parsed.expression)};
            }

            [[nodiscard]] std::optional<regex::omega::Expression> parse_alternation()
            {
                const std::size_t begin = position_;
                std::optional<regex::omega::Expression> first = parse_intersection();

                if (!first.has_value())
                {
                    position_ = begin;
                    return std::nullopt;
                }

                std::vector<regex::omega::Expression> alternatives;
                alternatives.push_back(std::move(*first));

                while (consume("|"))
                {
                    std::optional<regex::omega::Expression> next = parse_intersection();
                    if (!next.has_value())
                    {
                        record_error(position_, "expected an omega expression after '|'");
                        position_ = begin;
                        return std::nullopt;
                    }

                    alternatives.push_back(std::move(*next));
                }

                return regex::omega::make_alternation(std::move(alternatives));
            }

            [[nodiscard]] std::optional<regex::omega::Expression> parse_intersection()
            {
                const std::size_t begin = position_;
                std::optional<regex::omega::Expression> first = parse_concatenation();

                if (!first.has_value())
                {
                    position_ = begin;
                    return std::nullopt;
                }

                std::vector<regex::omega::Expression> operands;
                operands.push_back(std::move(*first));

                while (consume("&"))
                {
                    std::optional<regex::omega::Expression> next = parse_concatenation();
                    if (!next.has_value())
                    {
                        record_error(position_, "expected an omega expression after '&'");
                        position_ = begin;
                        return std::nullopt;
                    }

                    operands.push_back(std::move(*next));
                }

                return regex::omega::make_intersection(std::move(operands));
            }

            [[nodiscard]] std::optional<regex::omega::Expression> parse_concatenation()
            {
                const std::size_t begin = position_;
                std::vector<regex::Expression> prefixes;

                while (true)
                {
                    std::optional<FiniteMatch> prefix = finite_complement_at(position_);
                    if (!prefix.has_value())
                    {
                        break;
                    }

                    /*
                     * A finite candidate is a true prefix only if there is still
                     * input that can form the omega suffix.
                     *
                     * If it is followed by ^ω, it is the operand of an omega power.
                     * If it is followed by EOF, |, &, or ), it belongs to the omega
                     * expression at the current precedence level instead of being a
                     * prefix.
                     */
                    if (cannot_continue_finite_prefix(prefix->end))
                    {
                        break;
                    }

                    if (prefix->end == position_)
                    {
                        break;
                    }

                    position_ = prefix->end;
                    prefixes.push_back(std::move(prefix->expression));
                }

                std::optional<regex::omega::Expression> suffix = parse_complement();
                if (!suffix.has_value())
                {
                    position_ = begin;

                    if (starts_finite_expression(begin))
                    {
                        record_error(
                            begin,
                            "expected an omega regular expression; finite expressions must be "
                            "followed "
                            "by '^ω' or by an omega-language suffix"
                        );
                    }
                    else
                    {
                        record_error(begin, "expected an omega regular expression");
                    }

                    return std::nullopt;
                }

                regex::omega::Expression result = std::move(*suffix);
                for (auto iterator = prefixes.rbegin(); iterator != prefixes.rend(); ++iterator)
                {
                    result =
                        regex::omega::make_concatenation(std::move(*iterator), std::move(result));
                }

                return result;
            }

            [[nodiscard]] std::optional<regex::omega::Expression> parse_complement()
            {
                const std::size_t begin = position_;
                std::size_t complement_count = 0;

                while (consume("!") || consume("~"))
                {
                    ++complement_count;
                }

                std::optional<regex::omega::Expression> expression = parse_primary();
                if (!expression.has_value())
                {
                    position_ = begin;
                    return std::nullopt;
                }

                while (complement_count > 0)
                {
                    --complement_count;
                    expression = regex::omega::make_complement(std::move(*expression));
                }

                return expression;
            }

            [[nodiscard]] std::optional<regex::omega::Expression> parse_omega_power()
            {
                const std::size_t begin = position_;
                std::optional<FiniteMatch> finite = finite_repetition_at(begin);

                if (!finite.has_value())
                {
                    return std::nullopt;
                }

                position_ = finite->end;
                if (!consume("^ω"))
                {
                    position_ = begin;
                    return std::nullopt;
                }

                return regex::omega::make_omega_power(std::move(finite->expression));
            }

            [[nodiscard]] std::optional<regex::omega::Expression> parse_primary()
            {
                const std::size_t begin = position_;

                if (consume("Σ^ω"))
                {
                    return regex::omega::UniversalSet{};
                }

                if (consume("∅") && !starts_with(position_, "^ω"))
                {
                    return regex::omega::EmptySet{};
                }

                position_ = begin;
                if (std::optional<regex::omega::Expression> power = parse_omega_power())
                {
                    return power;
                }

                position_ = begin;
                if (consume("("))
                {
                    std::optional<regex::omega::Expression> expression = parse_alternation();
                    if (expression.has_value() && consume(")"))
                    {
                        return expression;
                    }

                    record_error(position_, "expected ')' after the omega expression");
                    position_ = begin;
                    return std::nullopt;
                }

                position_ = begin;
                return std::nullopt;
            }

            std::string_view input_;
            std::size_t position_ = 0;
            std::size_t error_offset_ = 0;
            std::string error_message_ = "invalid omega regular-expression syntax";
        };
    }

    ParseResult parse(const std::string_view input)
    {
        ParseResult result;

        const regex::parsing::detail::NormalizedInput normalized =
            regex::parsing::detail::normalize_with_mapping(input);

        Parser parser(normalized.text);
        result.expression = parser.parse();

        if (!result.expression.has_value())
        {
            const regex::parsing::ParseResult finite_parse =
                regex::parsing::parse(normalized.text);

            if (finite_parse.success())
            {
                result.error = regex::parsing::detail::make_parse_error(
                    input,
                    normalized.original_offset(0),
                    "expected an omega regular expression; finite expressions must be followed "
                    "by '^ω' or by an omega-language suffix"
                );
            }
            else
            {
                result.error = regex::parsing::detail::make_parse_error(
                    input,
                    normalized.original_offset(parser.error_offset()),
                    parser.error_message()
                );
            }
        }

        return result;
    }
}