// Declares configurable random regular-expression generation.
#pragma once

#include "regex/model/Expression.hpp"

#include <cstdint>
#include <random>
#include <string>

namespace regex::generation
{
    // Weights and depth bound used while generating an expression tree.
    struct Config
    {
        int stop_weight = 1;
        int continue_weight = 3;

        int empty_set_weight = 1;
        int epsilon_weight = 1;
        int letter_weight = 8;

        int kleene_star_weight = 2;
        int plus_weight = 2;
        int complement_weight = 1;
        int concatenation_weight = 4;
        int alternation_weight = 3;
        int intersection_weight = 1;

        int max_depth = 7;
    };

    // Clamps unsafe values and guarantees at least one usable generation choice.
    [[nodiscard]] Config sanitize_config(Config config);

    // Stateful pseudo-random regular-expression generator.
    class Generator
    {
    public:
        // Seeds the generator from the system random source.
        Generator();
        /// Seeds the generator deterministically for reproducible output.
        explicit Generator(std::uint32_t seed);

        // Generates an expression tree from a sanitized copy of the configuration.
        [[nodiscard]] Expression generate(const Config& config);
        // Generates and formats a parseable expression.
        [[nodiscard]] std::string generate_string(const Config& config);

    private:
        // Selects whether generation stops at a leaf or adds an operator.
        enum class GrowthChoice : std::uint8_t
        {
            Stop,
            Continue
        };

        // Selects the kind of generated atomic expression.
        enum class LeafChoice : std::uint8_t
        {
            EmptySet,
            Epsilon,
            Letter
        };

        // Selects the kind of generated compound expression.
        enum class OperatorChoice : std::uint8_t
        {
            KleeneStar,
            Plus,
            Complement,
            Concatenation,
            Alternation,
            Intersection
        };

        std::mt19937 random_;

        // Generates one subtree at the supplied depth.
        [[nodiscard]] Expression generate_expression(const Config& config, int depth);
        // Draws from the configured stop and continuation weights.
        [[nodiscard]] GrowthChoice choose_growth(const Config& config);
        // Draws from the configured leaf weights.
        [[nodiscard]] LeafChoice choose_leaf(const Config& config);
        // Draws from the configured operator weights.
        [[nodiscard]] OperatorChoice choose_operator(const Config& config);
        // Builds a leaf selected from the configured weights.
        [[nodiscard]] Expression generate_leaf(const Config& config);
        // Builds an operator node and its recursively generated children.
        [[nodiscard]] Expression generate_operator(const Config& config, int depth);
    };
}
