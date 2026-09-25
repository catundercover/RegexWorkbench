// Implements whitespace removal, aliases, and source-offset mapping.
#include "regex/parsing/InputNormalization.hpp"

#include <array>
#include <cctype>
#include <optional>
#include <utility>

namespace regex::parsing
{
    namespace
    {
        // Maps one supported Unicode superscript digit to its ASCII representation.
        [[nodiscard]] std::optional<std::pair<char, std::size_t>>
        superscript_digit_at(const std::string_view input, const std::size_t position)
        {
            constexpr std::array SuperscriptDigits{
                std::pair<std::string_view, char>{"⁰", '0'},
                std::pair<std::string_view, char>{"¹", '1'},
                std::pair<std::string_view, char>{"²", '2'},
                std::pair<std::string_view, char>{"³", '3'},
                std::pair<std::string_view, char>{"⁴", '4'},
                std::pair<std::string_view, char>{"⁵", '5'},
                std::pair<std::string_view, char>{"⁶", '6'},
                std::pair<std::string_view, char>{"⁷", '7'},
                std::pair<std::string_view, char>{"⁸", '8'},
                std::pair<std::string_view, char>{"⁹", '9'}
            };

            const std::string_view remaining = input.substr(position);
            for (const auto& [encoded, digit] : SuperscriptDigits)
            {
                if (remaining.starts_with(encoded))
                {
                    return std::pair{digit, encoded.size()};
                }
            }
            return std::nullopt;
        }
    }

    namespace detail
    {
        std::size_t NormalizedInput::original_offset(const std::size_t normalized_offset) const
        {
            return normalized_offset < source_offsets.size() ? source_offsets[normalized_offset]
                                                             : source_size;
        }

        NormalizedInput normalize_with_mapping(const std::string_view input)
        {
            NormalizedInput without_whitespace;
            without_whitespace.text.reserve(input.size());
            without_whitespace.source_offsets.reserve(input.size());
            without_whitespace.source_size = input.size();

            bool in_superscript_run = false;
            for (std::size_t index = 0; index < input.size(); ++index)
            {
                const auto byte = static_cast<unsigned char>(input[index]);
                if (std::isspace(byte))
                {
                    continue;
                }

                if (const auto superscript = superscript_digit_at(input, index))
                {
                    // A dead-key sequence may replace "^2" with "²" before ImGui receives it.
                    if (!in_superscript_run &&
                        (without_whitespace.text.empty() || without_whitespace.text.back() != '^'))
                    {
                        without_whitespace.text.push_back('^');
                        without_whitespace.source_offsets.push_back(index);
                    }
                    without_whitespace.text.push_back(superscript->first);
                    without_whitespace.source_offsets.push_back(index);
                    index += superscript->second - 1;
                    in_superscript_run = true;
                    continue;
                }

                in_superscript_run = false;
                without_whitespace.text.push_back(input[index]);
                without_whitespace.source_offsets.push_back(index);
            }

            NormalizedInput result;
            result.text.reserve(without_whitespace.text.size());
            result.source_offsets.reserve(without_whitespace.source_offsets.size());
            result.source_size = input.size();
            for (std::size_t index = 0; index < without_whitespace.text.size(); ++index)
            {
                const char current = without_whitespace.text[index];
                result.text.push_back(current);
                result.source_offsets.push_back(without_whitespace.source_offsets[index]);
                if ((current == '&' || current == '|') &&
                    index + 1 < without_whitespace.text.size() &&
                    without_whitespace.text[index + 1] == current)
                {
                    ++index;
                }
            }
            return result;
        }
    }

    std::string normalize_input(const std::string_view input)
    {
        return detail::normalize_with_mapping(input).text;
    }
}
