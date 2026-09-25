// Verifies coordination between asynchronous operations and UI state.
#include "ui/operations/OperationCoordinator.hpp"

#include "app/operations/OperationController.hpp"
#include "support/InlineOperationRunner.hpp"
#include "ui/comparison/ComparisonModel.hpp"
#include "ui/operations/OperationState.hpp"
#include "ui/results/ResultWidgets.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <variant>

namespace
{
    // Reports a failed assertion and returns its condition.
    bool check(bool condition, const std::string& message)
    {
        if (!condition)
        {
            std::cerr << message << '\n';
        }
        return condition;
    }
}

// Runs UI coordination and result-adaptation checks.
int main()
{
    app::operations::OperationController controller(
        std::make_unique<test_support::InlineOperationRunner>()
    );
    ui::operations::OperationCoordinator coordinator(controller);
    ui::operations::OperationState state;

    const std::chrono::steady_clock::time_point activity_start{};
    if (!check(
            !ui::results::activity_delay_elapsed(
                activity_start,
                activity_start + ui::results::ActivityIndicatorDelay - std::chrono::milliseconds(1)
            ),
            "The activity indicator became visible before its delay."
        ) ||
        !check(
            ui::results::activity_delay_elapsed(
                activity_start, activity_start + ui::results::ActivityIndicatorDelay
            ),
            "The activity indicator remained hidden after its delay."
        ))
    {
        return 1;
    }

    coordinator.start_translate(state, "", "");
    if (!check(
            std::holds_alternative<ui::operations::PendingOutput>(state.output),
            "Translation did not enter the pending state."
        ))
    {
        return 1;
    }

    coordinator.poll(state);
    const auto* failure = std::get_if<ui::operations::FailureOutput>(&state.output);
    if (!check(failure != nullptr, "Invalid translation did not produce a failure.") ||
        !check(
            failure->message == "Error: Please enter a regular expression.",
            "Invalid translation produced an unexpected message."
        ))
    {
        return 1;
    }

    coordinator.start_translate(state, "ε", "");
    coordinator.poll(state);
    const auto* epsilon_graph = std::get_if<ui::operations::GraphOutput>(&state.output);
    if (!check(
            epsilon_graph != nullptr,
            "Coordinator rejected epsilon with an empty active alphabet."
        ))
    {
        return 1;
    }

    coordinator.start_translate(state, "Σ", "");
    coordinator.poll(state);
    failure = std::get_if<ui::operations::FailureOutput>(&state.output);
    if (!check(failure != nullptr, "Coordinator accepted Sigma with an empty alphabet.") ||
        !check(
            failure->message == "Error: Alphabet is empty",
            "Coordinator produced an unexpected empty-alphabet error."
        ))
    {
        return 1;
    }

    state.rewrite_options.remove_plus = true;
    coordinator.start_rewrite(state, "a+", "");
    coordinator.poll(state);
    const auto* rewrite = std::get_if<ui::operations::RewriteOutput>(&state.output);
    if (!check(rewrite != nullptr, "Rewrite did not produce a rewrite output.") ||
        !check(!rewrite->regex.empty(), "Rewrite produced an empty expression.") ||
        !check(rewrite->regex.find('+') == std::string::npos, "Rewrite did not remove plus."))
    {
        return 1;
    }

    coordinator.start_compare(state, " a ", "a", "");
    coordinator.poll(state);
    const auto* comparison = std::get_if<ui::operations::ComparisonOutput>(&state.output);
    if (!check(comparison != nullptr, "Comparison did not produce a comparison output.") ||
        !check(
            comparison->comparison.kind == ui::comparison::RelationKind::Equivalent,
            "Equivalent expressions produced the wrong relation."
        ) ||
        !check(
            comparison->message == "The two regular expressions describe the same language.",
            "Comparison produced an unexpected description."
        ))
    {
        return 1;
    }

    coordinator.start_compare(state, " a ", " b ", "");
    coordinator.poll(state);
    comparison = std::get_if<ui::operations::ComparisonOutput>(&state.output);
    if (!check(comparison != nullptr, "Disjoint comparison did not produce an output.") ||
        !check(
            comparison->comparison.kind == ui::comparison::RelationKind::Disjoint,
            "Disjoint expressions produced the wrong relation."
        ) ||
        !check(
            comparison->comparison.only_left.has_value() &&
                comparison->comparison.only_left->label == "w1",
            "Comparison witnesses did not receive stable symbolic labels."
        ))
    {
        return 1;
    }

    state.flavor = app::operations::ExpressionFlavor::OmegaRegex;
    coordinator.start_translate(state, "∅", "");
    coordinator.poll(state);
    const auto* omega_empty_graph = std::get_if<ui::operations::GraphOutput>(&state.output);
    const auto* omega_empty_failure =
        std::get_if<ui::operations::FailureOutput>(&state.output);
    if (!check(
            omega_empty_graph != nullptr,
            std::string("Coordinator rejected omega empty set with an empty active alphabet") +
                (omega_empty_failure != nullptr ? ": " + omega_empty_failure->message : ".")
        ))
    {
        return 1;
    }

    coordinator.start_translate(state, "Σ^ω", "");
    coordinator.poll(state);
    failure = std::get_if<ui::operations::FailureOutput>(&state.output);
    if (!check(failure != nullptr, "Coordinator accepted Sigma omega with an empty alphabet.") ||
        !check(
            failure->message == "Error: Alphabet is empty",
            "Coordinator produced an unexpected omega empty-alphabet error."
        ))
    {
        return 1;
    }

    coordinator.start_translate(state, "∅ab^ω", "");
    coordinator.poll(state);
    const auto* omega_prefixed_empty_graph =
        std::get_if<ui::operations::GraphOutput>(&state.output);
    if (!check(
            omega_prefixed_empty_graph != nullptr,
            "Coordinator treated omega expression '∅ab^ω' as alphabet-empty."
        ))
    {
        return 1;
    }

    coordinator.start_translate(state, "a^ω", "a");
    coordinator.poll(state);
    const auto* omega_graph = std::get_if<ui::operations::GraphOutput>(&state.output);
    if (!check(omega_graph != nullptr, "Omega translation did not produce a graph output.") ||
        !check(!omega_graph->graph.nodes.empty(), "Omega translation produced an empty graph."))
    {
        return 1;
    }

    state.automaton_mode = ui::operations::AutomatonDisplayMode::Dfa;
    coordinator.start_translate(state, "(b*a)^ω", "ab");
    const auto* dba_pending = std::get_if<ui::operations::PendingOutput>(&state.output);
    if (!check(
            dba_pending != nullptr && dba_pending->label.find("DBA") != std::string::npos,
            "DBA translation did not identify the pending operation."
        ))
    {
        return 1;
    }
    coordinator.poll(state);
    const auto* dba_graph = std::get_if<ui::operations::GraphOutput>(&state.output);
    if (!check(
            dba_graph != nullptr && !dba_graph->graph.nodes.empty(),
            "DBA translation did not produce a graph."
        ))
    {
        return 1;
    }

    coordinator.start_translate(state, "(a|b)*a^ω", "ab");
    coordinator.poll(state);
    failure = std::get_if<ui::operations::FailureOutput>(&state.output);
    if (!check(
            failure != nullptr && failure->message.find("No deterministic Büchi automaton") == 0,
            "A non-DBA language did not produce the explicit nonexistence statement."
        ))
    {
        return 1;
    }
    state.automaton_mode = ui::operations::AutomatonDisplayMode::Nfa;
    coordinator.start_translate(state, "(a|b)*a^ω", "ab");
    coordinator.poll(state);
    if (!check(
            std::holds_alternative<ui::operations::GraphOutput>(state.output),
            "Switching back to NBA did not display the non-DBA language."
        ))
    {
        return 1;
    }

    coordinator.start_compare(state, "a^ω", "a^ω", "a");
    coordinator.poll(state);
    comparison = std::get_if<ui::operations::ComparisonOutput>(&state.output);
    if (!check(comparison != nullptr, "Omega comparison did not produce an output.") ||
        !check(
            comparison->comparison.kind == ui::comparison::RelationKind::Equivalent,
            "Equivalent omega expressions produced the wrong relation."
        ))
    {
        return 1;
    }

    state.rewrite_options.remove_plus = true;
    coordinator.start_rewrite(state, "(a+)^ω", "a");
    coordinator.poll(state);
    const auto* omega_rewrite = std::get_if<ui::operations::RewriteOutput>(&state.output);

    if (!check(omega_rewrite != nullptr, "Omega rewrite did not produce a rewrite output.") ||
        !check(
            omega_rewrite->regex == "(aa*)^ω", "Coordinator did not dispatch rewrite in omega mode."
        ))
    {
        return 1;
    }

    state.rewrite_options.remove_plus = false;
    state.flavor = app::operations::ExpressionFlavor::FiniteRegex;

    coordinator.clear(state);
    if (!check(
            std::holds_alternative<std::monostate>(state.output),
            "Clearing the coordinator did not reset its output."
        ))
    {
        return 1;
    }

    coordinator.start_translate(state, "a", "");
    coordinator.clear(state);
    coordinator.poll(state);
    if (!check(
            std::holds_alternative<std::monostate>(state.output),
            "Clearing a completed operation did not discard its queued result."
        ))
    {
        return 1;
    }

    return 0;
}
