/// @file
/// Defines persistent UI state and output variants for regex operations.
#pragma once

#include "app/operations/ExpressionFlavor.hpp"
#include "graph/model/Layout.hpp"
#include "ui/comparison/ComparisonModel.hpp"
#include "ui/graph/GraphCanvasModel.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <variant>

namespace ui::operations
{
    /// Selects the operation controls visible in the main window.
    enum class Mode : std::uint8_t
    {
        Translate,
        Rewrite,
        Compare
    };

    /// Selects the automaton representation requested by translation.
    enum class AutomatonDisplayMode : std::uint8_t
    {
        Nfa,
        Dfa
    };

    /// Stores the operator expansions selected by the user.
    struct RewriteOptions
    {
        bool remove_complement = false;
        bool remove_intersection = false;
        bool remove_power = false;
        bool remove_plus = false;
        bool remove_any_symbol = false;
    };

    /// Displays the label of an operation still in progress.
    struct PendingOutput
    {
        std::string label;
        std::chrono::steady_clock::time_point activity_started_at =
            std::chrono::steady_clock::now();
    };

    /// Displays a controlled operation failure.
    struct FailureOutput
    {
        std::string message;
    };

    /// Stores graph and copyable text views with their interaction state.
    struct GraphOutput
    {
        app::operations::ExpressionFlavor flavor = app::operations::ExpressionFlavor::FiniteRegex;
        ::graph::Layout original_graph;
        ::graph::Layout graph;
        std::string automaton_text;
        ::ui::graph::GraphCanvasState canvas;
        bool text_view_visible = false;
    };

    /// Stores a rewrite summary and the parseable rewritten expression.
    struct RewriteOutput
    {
        app::operations::ExpressionFlavor flavor = app::operations::ExpressionFlavor::FiniteRegex;
        std::string regex;
    };

    /// Stores a comparison summary and diagram-ready model.
    struct ComparisonOutput
    {
        app::operations::ExpressionFlavor flavor = app::operations::ExpressionFlavor::FiniteRegex;
        std::string message;
        comparison::ComparisonModel comparison;
    };

    /// Complete set of operation states rendered by the result view.
    using OperationOutput = std::variant<
        std::monostate,
        PendingOutput,
        FailureOutput,
        GraphOutput,
        RewriteOutput,
        ComparisonOutput>;

    /// Persistent state shared by operation controls and result presentation.
    struct OperationState
    {
        Mode mode = Mode::Translate;
        app::operations::ExpressionFlavor flavor = app::operations::ExpressionFlavor::FiniteRegex;
        AutomatonDisplayMode automaton_mode = AutomatonDisplayMode::Nfa;
        RewriteOptions rewrite_options;
        OperationOutput output;
    };
}
