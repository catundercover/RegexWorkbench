// Implements weighted, depth-bounded random regex generation.
#include "regex/generation/RandomRegexGenerator.hpp"

#include "regex/formatting/RegexFormatter.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

namespace regex::generation
{
    namespace
    {
        // Smallest supported generated-tree depth.
        constexpr int MinimumDepth = 1;
        // Largest supported generated-tree depth.
        constexpr int MaximumDepth = 10;

        // Draws one choice in proportion to its positive integer weight.
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

        // Clamps one generation weight to the supported nonnegative range.
        int nonnegative(const int value)
        {
            return std::max(value, 0);
        }
    }

    Generator::Generator() : random_(std::random_device{}())
    {
    }

    Generator::Generator(const std::uint32_t seed) : random_(seed)
    {
    }

    Expression Generator::generate(const Config& config)
    {
        return generate_expression(sanitize_config(config), 0);
    }

    std::string Generator::generate_string(const Config& config)
    {
        return formatting::format(generate(config));
    }

    Expression Generator::generate_expression(const Config& config, const int depth)
    {
        if (depth >= config.max_depth || choose_growth(config) == GrowthChoice::Stop)
        {
            return generate_leaf(config);
        }
        return generate_operator(config, depth);
    }

    Generator::GrowthChoice Generator::choose_growth(const Config& config)
    {
        return choose_weighted<GrowthChoice>(
            random_,
            std::array{
                std::pair{GrowthChoice::Stop, config.stop_weight},
                std::pair{GrowthChoice::Continue, config.continue_weight}
            }
        );
    }

    Generator::LeafChoice Generator::choose_leaf(const Config& config)
    {
        return choose_weighted<LeafChoice>(
            random_,
            std::array{
                std::pair{LeafChoice::EmptySet, config.empty_set_weight},
                std::pair{LeafChoice::Epsilon, config.epsilon_weight},
                std::pair{LeafChoice::Letter, config.letter_weight}
            }
        );
    }

    Generator::OperatorChoice Generator::choose_operator(const Config& config)
    {
        return choose_weighted<OperatorChoice>(
            random_,
            std::array{
                std::pair{OperatorChoice::KleeneStar, config.kleene_star_weight},
                std::pair{OperatorChoice::Plus, config.plus_weight},
                std::pair{OperatorChoice::Complement, config.complement_weight},
                std::pair{OperatorChoice::Concatenation, config.concatenation_weight},
                std::pair{OperatorChoice::Alternation, config.alternation_weight},
                std::pair{OperatorChoice::Intersection, config.intersection_weight}
            }
        );
    }

    Expression Generator::generate_leaf(const Config& config)
    {
        switch (choose_leaf(config))
        {
        case LeafChoice::EmptySet:
            return EmptySet{};
        case LeafChoice::Epsilon:
            return Epsilon{};
        case LeafChoice::Letter:
        {
            std::uniform_int_distribution<int> distribution(0, 1);
            return make_terminal(distribution(random_) == 0 ? 'a' : 'b');
        }
        }
        return make_terminal('a');
    }

    Expression Generator::generate_operator(const Config& config, const int depth)
    {
        switch (choose_operator(config))
        {
        case OperatorChoice::KleeneStar:
            return make_star(generate_expression(config, depth + 1));
        case OperatorChoice::Plus:
            return make_plus(generate_expression(config, depth + 1));
        case OperatorChoice::Complement:
            return make_complement(generate_expression(config, depth + 1));
        case OperatorChoice::Concatenation:
            return make_concatenation(
                {generate_expression(config, depth + 1), generate_expression(config, depth + 1)}
            );
        case OperatorChoice::Alternation:
            return make_alternation(
                {generate_expression(config, depth + 1), generate_expression(config, depth + 1)}
            );
        case OperatorChoice::Intersection:
            return make_intersection(
                {generate_expression(config, depth + 1), generate_expression(config, depth + 1)}
            );
        }
        return generate_leaf(config);
    }

    Config sanitize_config(Config config)
    {
        config.stop_weight = nonnegative(config.stop_weight);
        config.continue_weight = nonnegative(config.continue_weight);
        config.empty_set_weight = nonnegative(config.empty_set_weight);
        config.epsilon_weight = nonnegative(config.epsilon_weight);
        config.letter_weight = nonnegative(config.letter_weight);
        config.kleene_star_weight = nonnegative(config.kleene_star_weight);
        config.plus_weight = nonnegative(config.plus_weight);
        config.complement_weight = nonnegative(config.complement_weight);
        config.concatenation_weight = nonnegative(config.concatenation_weight);
        config.alternation_weight = nonnegative(config.alternation_weight);
        config.intersection_weight = nonnegative(config.intersection_weight);
        config.max_depth = std::clamp(config.max_depth, MinimumDepth, MaximumDepth);

        if (config.empty_set_weight == 0 && config.epsilon_weight == 0 && config.letter_weight == 0)
        {
            config.letter_weight = 1;
        }
        if (config.kleene_star_weight == 0 && config.plus_weight == 0 &&
            config.complement_weight == 0 && config.concatenation_weight == 0 &&
            config.alternation_weight == 0 && config.intersection_weight == 0)
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
