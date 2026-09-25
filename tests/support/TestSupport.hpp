// Defines lightweight assertions and regex fixtures shared by tests.
#pragma once

#include "regex/model/Expression.hpp"
#include "regex/model/OmegaExpression.hpp"
#include "regex/parsing/OmegaRegexParser.hpp"
#include "regex/parsing/RegexParser.hpp"

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace test_support
{
    // Reports a failed condition and returns it for aggregate test status.
    inline bool check(const bool condition, const std::string_view message)
    {
        if (!condition)
        {
            std::cerr << message << '\n';
        }
        return condition;
    }

    // Parses a required test expression or throws when the fixture is invalid.
    inline regex::Expression parse_expression(const std::string_view text)
    {
        regex::parsing::ParseResult result = regex::parsing::parse(text);
        if (!result.expression.has_value() || result.error.has_value())
        {
            throw std::runtime_error("Test expression did not parse: " + std::string(text));
        }
        return std::move(result.expression).value();
    }

    // Parses a required omega test expression or throws when the fixture is invalid.
    inline regex::omega::Expression parse_omega_expression(const std::string_view text)
    {
        regex::parsing::omega::ParseResult result = regex::parsing::omega::parse(text);
        if (!result.expression.has_value() || result.error.has_value())
        {
            throw std::runtime_error("Test omega expression did not parse: " + std::string(text));
        }
        return std::move(result.expression).value();
    }

    // Recursively appends every word extending the current prefix.
    inline void append_words(
        std::vector<std::string>& words,
        std::string& prefix,
        const std::string_view alphabet,
        const std::size_t remaining_length
    )
    {
        words.push_back(prefix);
        if (remaining_length == 0)
        {
            return;
        }

        for (const char symbol : alphabet)
        {
            prefix.push_back(symbol);
            append_words(words, prefix, alphabet, remaining_length - 1);
            prefix.pop_back();
        }
    }

    // Enumerates all words over an alphabet through the requested length.
    inline std::vector<std::string>
    words_through_length(const std::string_view alphabet, const std::size_t maximum_length)
    {
        std::vector<std::string> words;
        std::string prefix;
        append_words(words, prefix, alphabet, maximum_length);
        return words;
    }
}
