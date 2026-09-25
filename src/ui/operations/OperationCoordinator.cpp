/// @file
/// Implements request construction and operation-result adaptation for the UI.
#include "ui/operations/OperationCoordinator.hpp"

#include "app/operations/OperationController.hpp"
#include "ui/comparison/ComparisonModel.hpp"
#include "ui/graph/AutomatonText.hpp"
#include "ui/operations/OperationState.hpp"

#include <optional>
#include <string>
#include <utility>
#include <variant>

namespace ui::operations
{
    namespace
    {
        /// Combines lambdas into one visitor for result dispatch.
        template <typename... Visitors>
        struct Overloaded : Visitors...
        {
            using Visitors::operator()...;
        };

        /// Adapts a semantic comparison for witness-based presentation.
        ComparisonOutput make_comparison_output(
            const app::operations::CompareResult& result,
            const app::operations::ExpressionFlavor flavor
        )
        {
            comparison::ComparisonModel model = std::visit(
                [](const auto& comparison)
                { return comparison::make_comparison_model(comparison); },
                result.comparison
            );

            return ComparisonOutput{flavor, comparison::describe_relation(model), std::move(model)};
        }
    }

    OperationCoordinator::OperationCoordinator(app::operations::OperationController& controller)
        : controller_(controller)
    {
    }

    void OperationCoordinator::clear(OperationState& state)
    {
        controller_.cancel();
        pending_kind_ = PendingKind::None;
        state.output = std::monostate{};
    }

    void OperationCoordinator::cancel(OperationState& state)
    {
        clear(state);
    }

    void OperationCoordinator::poll(OperationState& state)
    {
        controller_.poll();
        std::optional<app::operations::OperationResult> result = controller_.take_result();
        if (!result.has_value())
        {
            return;
        }

        pending_kind_ = PendingKind::None;

        state.output = std::visit(
            Overloaded{
                [](app::operations::OperationFailure failure) -> OperationOutput
                { return FailureOutput{std::move(failure.message)}; },
                [&state](app::operations::TranslateResult translation) -> OperationOutput
                {
                    GraphOutput output;
                    output.flavor = state.flavor;
                    output.original_graph = translation.graph_layout;
                    output.graph = std::move(translation.graph_layout);
                    output.automaton_text = ::ui::graph::format_automaton_text(output.graph);
                    return output;
                },
                [&state](app::operations::RewriteResult rewrite) -> OperationOutput
                { return RewriteOutput{state.flavor, std::move(rewrite.rewritten_regex)}; },
                [&state](const app::operations::CompareResult& comparison) -> OperationOutput
                { return make_comparison_output(comparison, state.flavor); }
            },
            std::move(*result)
        );
    }

    void OperationCoordinator::start_translate(
        OperationState& state, std::string_view regex, std::string_view extra_alphabet
    )
    {
        const bool omega = state.flavor == app::operations::ExpressionFlavor::OmegaRegex;
        const bool use_dfa = state.automaton_mode == AutomatonDisplayMode::Dfa;

        begin(
            state,
            omega ? (use_dfa ? "Computing minimal state-based DBA..."
                             : "Computing state-based NBA...")
                  : (use_dfa ? "Computing minimal DFA..." : "Computing NFA...")
        );

        app::operations::TranslateRequest input;
        input.regex = regex;
        input.flavor = state.flavor;
        input.mode =
            use_dfa ? app::operations::TranslateMode::Dfa : app::operations::TranslateMode::Nfa;
        input.extra_alphabet = extra_alphabet;

        if (!controller_.start(std::move(input)))
        {
            pending_kind_ = PendingKind::None;
            state.output = FailureOutput{"Could not start translation."};
        }
    }

    void OperationCoordinator::start_rewrite(
        OperationState& state, std::string_view regex, std::string_view extra_alphabet
    )
    {
        begin(
            state,
            state.flavor == app::operations::ExpressionFlavor::OmegaRegex
                ? "Rewriting omega regular expression..."
                : "Rewriting regular expression..."
        );

        app::operations::RewriteRequest input;
        input.regex = regex;
        input.flavor = state.flavor;
        input.extra_alphabet = extra_alphabet;
        input.remove_complement = state.rewrite_options.remove_complement;
        input.remove_intersection = state.rewrite_options.remove_intersection;
        input.remove_power = state.rewrite_options.remove_power;
        input.remove_plus = state.rewrite_options.remove_plus;
        input.remove_any_symbol = state.rewrite_options.remove_any_symbol;

        if (!controller_.start(std::move(input)))
        {
            pending_kind_ = PendingKind::None;
            state.output = FailureOutput{"Could not start rewrite."};
        }
    }

    void OperationCoordinator::start_compare(
        OperationState& state,
        std::string_view left_regex,
        std::string_view right_regex,
        std::string_view extra_alphabet
    )
    {
        begin(
            state,
            state.flavor == app::operations::ExpressionFlavor::OmegaRegex
                ? "Comparing omega regular expressions..."
                : "Comparing regular expressions..."
        );

        app::operations::CompareRequest input;
        input.flavor = state.flavor;
        input.left_regex = left_regex;
        input.right_regex = right_regex;
        input.extra_alphabet = extra_alphabet;

        if (!controller_.start(std::move(input)))
        {
            pending_kind_ = PendingKind::None;
            state.output = FailureOutput{"Could not start comparison."};
        }
    }

    bool OperationCoordinator::running() const
    {
        return controller_.is_running();
    }

    void OperationCoordinator::begin(OperationState& state, std::string label)
    {
        clear(state);
        state.output = PendingOutput{std::move(label)};
        pending_kind_ = PendingKind::PrimaryOperation;
    }
}
