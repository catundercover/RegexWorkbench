// Declares validation helpers shared by operation implementations.
#pragma once

#include "automata/model/BuchiAutomaton.hpp"
#include "automata/model/Nfa.hpp"
#include "regex/model/Expression.hpp"
#include "regex/model/OmegaExpression.hpp"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_set>

namespace app::operations
{
    // Result of parsing the optional user-supplied alphabet symbols.
    struct AlphabetParseResult
    {
        std::unordered_set<char> symbols;
        std::optional<std::string> error;

        /// Returns whether parsing completed without a validation error.
        [[nodiscard]] bool valid() const noexcept
        {
            return !error.has_value();
        }
    };

    // Parses alphanumeric symbols while ignoring whitespace.
    [[nodiscard]] AlphabetParseResult parse_extra_alphabet(std::string_view input);

    namespace detail
    {
        // Parses a required finite regular expression or throws a labeled user-facing error.
        [[nodiscard]] regex::Expression  require_regex(std::string_view raw_regex,
            std::string_view label);

        // Parses a required omega regular expression or throws a labeled user-facing error.
        [[nodiscard]] regex::omega::Expression require_omega_regex(std::string_view raw_regex,
            std::string_view label);

        // Parses an optional alphabet or throws its user-facing validation error.
        [[nodiscard]] automata::Alphabet require_extra_alphabet(std::string_view input);

        // Rejects finite-regex operations that reference Σ or alphabet-relative operators while
        // the expression symbols and explicit symbols form an empty alphabet.
        void require_nonempty_alphabet(std::span<const regex::Expression* const> expressions,
            const automata::Alphabet& extra_alphabet);

        // Rejects omega-regex operations that reference Σ^ω, Σ, or alphabet-relative operators
        // while the expression symbols and explicit symbols form an empty alphabet.
        void require_nonempty_alphabet(std::span<const regex::omega::Expression* const> expressions,
            const automata::Alphabet& extra_alphabet);

        // Counts individual labeled transitions as they will appear in an NFA/DFA graph.
        [[nodiscard]] std::size_t rendered_transition_count(const automata::Nfa& automaton);

        // Counts individual labeled transitions as they will appear in a Büchi graph.
        [[nodiscard]] std::size_t rendered_transition_count(const automata::BuchiAutomaton& automaton);

        // Rejects finite automata that exceed the configured graph complexity limits.
        void require_renderable(const automata::Nfa& automaton);

        // Rejects Büchi automata that exceed the configured graph complexity limits.
        void require_renderable(const automata::BuchiAutomaton& automaton);

        // Rejects DOT documents that exceed the layout transport limit.
        void require_dot_size(std::string_view dot);
    }
}