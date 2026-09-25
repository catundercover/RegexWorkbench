// Implements weighted, depth-bounded random omega-regex generation.
#include "regex/generation/RandomOmegaRegexGenerator.hpp"

#include "regex/formatting/OmegaRegexFormatter.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <random>
#include <utility>

namespace regex::generation
{
    namespace
    {
        constexpr int MinimumOmegaDepth = 1;
        constexpr int MaximumOmegaDepth = 10;

        template <typename Choice, std::size_t Size>
        Choice choose_weighted(
            std::mt19937& random, const std::array<std::pair<Choice, int>, Size>& weighted_choices
        )
        {
            std::uint64_t total_weight = 0;
            for (const auto& [choice, weight] : weighted_choices)
            {
                (void)choice;
                if (weight > 0)
                {
                    total_weight += static_cast<std::uint64_t>(weight);
                }
            }

            std::uniform_int_distribution<std::uint64_t> distribution(1, total_weight);
            std::uint64_t roll = distribution(random);

            for (const auto& [choice, weight] : weighted_choices)
            {
                if (weight <= 0)
                {
                    continue;
                }

                const auto positive_weight = static_cast<std::uint64_t>(weight);
                if (roll <= positive_weight)
                {
                    return choice;
                }
                roll -= positive_weight;
            }

            return weighted_choices.back().first;
        }

        int nonnegative(const int value)
        {
            return std::max(value, 0);
        }

        Config make_non_empty_period_config(Config config)
        {
            config = sanitize_config(config);

            config.empty_set_weight = 0;
            config.epsilon_weight = 0;

            if (config.letter_weight == 0)
            {
                config.letter_weight = 1;
            }

            return sanitize_config(config);
        }
    }

    OmegaGenerator::OmegaGenerator()
        : random_(std::random_device{}()), finite_generator_(std::random_device{}())
    {
    }

    OmegaGenerator::OmegaGenerator(const std::uint32_t seed)
        : random_(seed), finite_generator_(seed + 1)
    {
    }

    omega::Expression OmegaGenerator::generate(const OmegaConfig& config)
    {
        return generate_expression(sanitize_omega_config(config), 0);
    }

    std::string OmegaGenerator::generate_string(const OmegaConfig& config)
    {
        return formatting::omega::format(generate(config));
    }

    omega::Expression OmegaGenerator::generate_expression(const OmegaConfig& config, const int depth)
    {
        if (depth >= config.max_depth || choose_growth(config) == GrowthChoice::Stop)
        {
            return generate_leaf(config);
        }

        return generate_operator(config, depth);
    }

    OmegaGenerator::GrowthChoice OmegaGenerator::choose_growth(const OmegaConfig& config)
    {
        return choose_weighted<GrowthChoice>(
            random_,
            std::array{
                std::pair{GrowthChoice::Stop, config.stop_weight},
                std::pair{GrowthChoice::Continue, config.continue_weight}
            }
        );
    }

    OmegaGenerator::LeafChoice OmegaGenerator::choose_leaf(const OmegaConfig& config)
    {
        return choose_weighted<LeafChoice>(
            random_,
            std::array{
                std::pair{LeafChoice::EmptySet, config.empty_set_weight},
                std::pair{LeafChoice::UniversalSet, config.universal_set_weight},
                std::pair{LeafChoice::OmegaPower, config.omega_power_weight}
            }
        );
    }

    OmegaGenerator::OperatorChoice OmegaGenerator::choose_operator(const OmegaConfig& config)
    {
        return choose_weighted<OperatorChoice>(
            random_,
            std::array{
                std::pair{OperatorChoice::PrefixConcat, config.prefix_concat_weight},
                std::pair{OperatorChoice::Alternation, config.alternation_weight},
                std::pair{OperatorChoice::Intersection, config.intersection_weight},
                std::pair{OperatorChoice::Complement, config.complement_weight}
            }
        );
    }

    omega::Expression OmegaGenerator::generate_leaf(const OmegaConfig& config)
    {
        switch (choose_leaf(config))
        {
        case LeafChoice::EmptySet:
            return omega::EmptySet{};
        case LeafChoice::UniversalSet:
            return omega::UniversalSet{};
        case LeafChoice::OmegaPower:
            return omega::make_omega_power(finite_generator_.generate(config.period_config));
        }

        return omega::make_omega_power(finite_generator_.generate(config.period_config));
    }

    omega::Expression OmegaGenerator::generate_operator(const OmegaConfig& config, const int depth)
    {
        switch (choose_operator(config))
        {
        case OperatorChoice::PrefixConcat:
        {
            const regex::Expression prefix = finite_generator_.generate(config.prefix_config);
            const regex::Expression period =
                finite_generator_.generate(make_non_empty_period_config(config.period_config));

            return omega::make_concatenation(prefix, omega::make_omega_power(period));
        }
        case OperatorChoice::Alternation:
            return omega::make_alternation(
                {generate_expression(config, depth + 1), generate_expression(config, depth + 1)}
            );
        case OperatorChoice::Intersection:
            return omega::make_intersection(
                {generate_expression(config, depth + 1), generate_expression(config, depth + 1)}
            );
        case OperatorChoice::Complement:
            return omega::make_complement(generate_expression(config, depth + 1));
        }

        return generate_leaf(config);
    }

    OmegaConfig sanitize_omega_config(OmegaConfig config)
    {
        config.stop_weight = nonnegative(config.stop_weight);
        config.continue_weight = nonnegative(config.continue_weight);

        config.empty_set_weight = nonnegative(config.empty_set_weight);
        config.universal_set_weight = nonnegative(config.universal_set_weight);
        config.omega_power_weight = nonnegative(config.omega_power_weight);

        config.prefix_concat_weight = nonnegative(config.prefix_concat_weight);
        config.alternation_weight = nonnegative(config.alternation_weight);
        config.intersection_weight = nonnegative(config.intersection_weight);
        config.complement_weight = nonnegative(config.complement_weight);

        config.max_depth = std::clamp(config.max_depth, MinimumOmegaDepth, MaximumOmegaDepth);

        config.prefix_config = sanitize_config(config.prefix_config);
        config.period_config = sanitize_config(config.period_config);

        if (config.empty_set_weight == 0 && config.universal_set_weight == 0 &&
            config.omega_power_weight == 0)
        {
            config.omega_power_weight = 1;
        }

        if (config.prefix_concat_weight == 0 && config.alternation_weight == 0 &&
            config.intersection_weight == 0 && config.complement_weight == 0)
        {
            config.continue_weight = 0;
        }

        if (config.stop_weight == 0 && config.continue_weight == 0)
        {
            config.stop_weight = 1;
        }

        return config;
    }
}