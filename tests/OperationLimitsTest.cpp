// Verifies input, output, and graph-complexity operation limits.
#include "app/operations/OperationLimits.hpp"

#include "app/operations/OperationExecutor.hpp"
#include "app/operations/OperationValidation.hpp"
#include "support/TestSupport.hpp"

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace
{
    // Runs an operation and checks that its runtime error contains expected text.
    template <typename Operation>
    bool throws_with(Operation&& operation, const std::string_view expected_text)
    {
        try
        {
            std::forward<Operation>(operation)();
        }
        catch (const std::runtime_error& error)
        {
            return std::string_view(error.what()).find(expected_text) != std::string_view::npos;
        }
        return false;
    }

    // Builds a valid automaton with the requested number of isolated states.
    automata::Nfa automaton_with_states(const std::size_t count)
    {
        automata::Nfa automaton;
        for (std::size_t state = 0; state < count; ++state)
        {
            (void)automaton.add_state();
        }
        return automaton;
    }
}

// Runs configured regex, graph, and DOT limit checks.
int main()
{
    using namespace app::operations;

    const std::string oversized_regex(limits::MaxRegexInputBytes + 1, 'a');
    const OperationResult oversized_result = execute(
        TranslateRequest{oversized_regex, ExpressionFlavor::FiniteRegex, TranslateMode::Nfa, ""}
    );
    const auto* oversized_failure = std::get_if<OperationFailure>(&oversized_result);
    if (!test_support::check(
            oversized_failure != nullptr &&
                oversized_failure->message.find("Maximum length") != std::string::npos,
            "Oversized regex input did not produce the configured limit error."
        ))
    {
        return 1;
    }

           const regex::Expression epsilon = test_support::parse_expression("ε");
        const regex::Expression empty_set = test_support::parse_expression("∅");
        const regex::Expression sigma = test_support::parse_expression("Σ");
        const regex::Expression sigma_power = test_support::parse_expression("Σ^2");
        const regex::Expression epsilon_sigma = test_support::parse_expression("εΣ");
        const regex::Expression terminal = test_support::parse_expression("a");

        const std::array epsilon_only{&epsilon};
        const std::array empty_set_only{&empty_set};
        const std::array sigma_only{&sigma};
        const std::array sigma_power_only{&sigma_power};
        const std::array epsilon_sigma_only{&epsilon_sigma};
        const std::array terminal_only{&terminal};

        detail::require_nonempty_alphabet(epsilon_only, {});
        detail::require_nonempty_alphabet(empty_set_only, {});
        detail::require_nonempty_alphabet(epsilon_only, {'a'});
        detail::require_nonempty_alphabet(terminal_only, {});

        if (!test_support::check(
                throws_with(
                    [&] { detail::require_nonempty_alphabet(sigma_only, {}); },
                    "Alphabet is empty"
                ),
                "Sigma over an empty active alphabet was accepted."
            ) ||
            !test_support::check(
                throws_with(
                    [&] { detail::require_nonempty_alphabet(sigma_power_only, {}); },
                    "Alphabet is empty"
                ),
                "Sigma power over an empty active alphabet was accepted."
            ) ||
            !test_support::check(
                throws_with(
                    [&] { detail::require_nonempty_alphabet(epsilon_sigma_only, {}); },
                    "Alphabet is empty"
                ),
                "Epsilon followed by Sigma over an empty active alphabet was accepted."
            ))
        {
            return 1;
        }

        detail::require_nonempty_alphabet(sigma_only, {'a'});
        detail::require_nonempty_alphabet(sigma_power_only, {'a'});
        detail::require_nonempty_alphabet(epsilon_sigma_only, {'a'});

        const regex::omega::Expression omega_empty = test_support::parse_omega_expression("∅");
        const regex::omega::Expression omega_sigma = test_support::parse_omega_expression("Σ^ω");
        const regex::omega::Expression omega_terminal = test_support::parse_omega_expression("a^ω");
        const regex::omega::Expression omega_empty_prefixed_terminals =
            test_support::parse_omega_expression("∅ab^ω");

        const std::array omega_empty_only{&omega_empty};
        const std::array omega_sigma_only{&omega_sigma};
        const std::array omega_terminal_only{&omega_terminal};
        const std::array omega_empty_prefixed_terminals_only{&omega_empty_prefixed_terminals};

        detail::require_nonempty_alphabet(omega_empty_only, {});
        detail::require_nonempty_alphabet(omega_empty_only, {'a'});
        detail::require_nonempty_alphabet(omega_terminal_only, {});
        detail::require_nonempty_alphabet(omega_empty_prefixed_terminals_only, {});

        if (!test_support::check(
                throws_with(
                    [&] { detail::require_nonempty_alphabet(omega_sigma_only, {}); },
                    "Alphabet is empty"
                ),
                "Omega Sigma over an empty active alphabet was accepted."
            ))
        {
            return 1;
        }

        detail::require_nonempty_alphabet(omega_sigma_only, {'a'});

        automata::Nfa maximum_states = automaton_with_states(limits::MaxRenderedStates);
    detail::require_renderable(maximum_states);
    (void)maximum_states.add_state();
    if (!test_support::check(
            throws_with(
                [&] { detail::require_renderable(maximum_states); }, "too many states to render"
            ),
            "An automaton above the render-state limit was accepted."
        ))
    {
        return 1;
    }

    automata::BuchiAutomaton maximum_buchi_states;
    maximum_buchi_states.start = 0;
    maximum_buchi_states.transitions.resize(limits::MaxRenderedStates);
    detail::require_renderable(maximum_buchi_states);
    maximum_buchi_states.transitions.emplace_back();
    if (!test_support::check(
            throws_with(
                [&] { detail::require_renderable(maximum_buchi_states); },
                "too many states to render"
            ),
            "A Büchi automaton above the render-state limit was accepted."
        ))
    {
        return 1;
    }

    automata::Nfa maximum_transitions = automaton_with_states(limits::MaxRenderedStates);
    std::size_t rendered_edges = 1;
    for (automata::StateId source = 0; source < maximum_transitions.transitions.size() &&
                                       rendered_edges < limits::MaxRenderedTransitions;
         ++source)
    {
        for (automata::StateId target = 0; target < maximum_transitions.transitions.size() &&
                                           rendered_edges < limits::MaxRenderedTransitions;
             ++target)
        {
            maximum_transitions.add_transition(source, 'a', target);
            ++rendered_edges;
        }
    }
    detail::require_renderable(maximum_transitions);
    maximum_transitions.add_transition(4, 'a', 0);
    if (!test_support::check(
            throws_with(
                [&] { detail::require_renderable(maximum_transitions); },
                "too many transitions to render"
            ),
            "An automaton above the render-transition limit was accepted."
        ))
    {
        return 1;
    }

    const std::string maximum_dot(limits::MaxDotBytes, 'x');
    detail::require_dot_size(maximum_dot);
    if (!test_support::check(
            throws_with(
                [&] { detail::require_dot_size(maximum_dot + 'x'); },
                "graph is too large to display"
            ),
            "DOT data above the configured size limit was accepted."
        ))
    {
        return 1;
    }

    return 0;
}
