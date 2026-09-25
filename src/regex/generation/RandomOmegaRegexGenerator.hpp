// Declares configurable random omega-regular-expression generation.
#pragma once

#include "regex/generation/RandomRegexGenerator.hpp"
#include "regex/model/OmegaExpression.hpp"

#include <cstdint>
#include <random>
#include <string>

namespace regex::generation
{
    // Weights and depth bounds used while generating omega-regular expressions.
    struct OmegaConfig
    {
        int stop_weight = 1;
        int continue_weight = 3;

        int empty_set_weight = 1;
        int universal_set_weight = 1;
        int omega_power_weight = 6;

        int prefix_concat_weight = 4;
        int alternation_weight = 4;
        int intersection_weight = 1;
        int complement_weight = 1;

        int max_depth = 5;

        Config prefix_config{
            .stop_weight = 1,
            .continue_weight = 2,
            .empty_set_weight = 1,
            .epsilon_weight = 2,
            .letter_weight = 8,
            .kleene_star_weight = 2,
            .plus_weight = 1,
            .complement_weight = 0,
            .concatenation_weight = 4,
            .alternation_weight = 3,
            .intersection_weight = 0,
            .max_depth = 4
        };

        Config period_config{
            .stop_weight = 1,
            .continue_weight = 2,
            .empty_set_weight = 0,
            .epsilon_weight = 0,
            .letter_weight = 10,
            .kleene_star_weight = 1,
            .plus_weight = 2,
            .complement_weight = 0,
            .concatenation_weight = 4,
            .alternation_weight = 3,
            .intersection_weight = 0,
            .max_depth = 4
        };
    };

    // Clamps unsafe omega-generator values and guarantees usable choices.
    [[nodiscard]] OmegaConfig sanitize_omega_config(OmegaConfig config);

    // Stateful pseudo-random omega-regular-expression generator.
    class OmegaGenerator
    {
    public:
        // Seeds the generator from the system random source.
        OmegaGenerator();

        // Seeds the generator deterministically for reproducible output.
        explicit OmegaGenerator(std::uint32_t seed);

        // Generates an omega-expression tree from a sanitized copy of the configuration.
        [[nodiscard]] omega::Expression generate(const OmegaConfig& config);

        // Generates and formats a parseable omega expression.
        [[nodiscard]] std::string generate_string(const OmegaConfig& config);

    private:
        enum class GrowthChoice : std::uint8_t
        {
            Stop,
            Continue
        };

        enum class LeafChoice : std::uint8_t
        {
            EmptySet,
            UniversalSet,
            OmegaPower
        };

        enum class OperatorChoice : std::uint8_t
        {
            PrefixConcat,
            Alternation,
            Intersection,
            Complement
        };

        std::mt19937 random_;
        Generator finite_generator_;

        [[nodiscard]] omega::Expression generate_expression(const OmegaConfig& config, int depth);
        [[nodiscard]] GrowthChoice choose_growth(const OmegaConfig& config);
        [[nodiscard]] LeafChoice choose_leaf(const OmegaConfig& config);
        [[nodiscard]] OperatorChoice choose_operator(const OmegaConfig& config);
        [[nodiscard]] omega::Expression generate_leaf(const OmegaConfig& config);
        [[nodiscard]] omega::Expression generate_operator(const OmegaConfig& config, int depth);
    };
}