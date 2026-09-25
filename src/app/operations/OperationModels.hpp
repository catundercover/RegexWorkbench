// Defines the requests and results exchanged with isolated operation workers.
#pragma once

#include "app/operations/ExpressionFlavor.hpp"
#include "automata/analysis/OmegaRegexComparison.hpp"
#include "automata/analysis/RegexComparison.hpp"
#include "graph/model/Layout.hpp"

#include <cstdint>
#include <string>
#include <variant>

namespace app::operations
{
    // Selects nondeterministic or minimal deterministic translation:
    // NFA/DFA for finite words, state-based NBA/DBA for infinite words.
    enum class TranslateMode : std::uint8_t
    {
        Nfa,
        Dfa
    };

    // Requests regex translation and graph layout.
    struct TranslateRequest
    {
        std::string regex;
        ExpressionFlavor flavor = ExpressionFlavor::FiniteRegex;
        TranslateMode mode = TranslateMode::Nfa;
        std::string extra_alphabet;
    };

    // Requests normalization and selected operator expansions.
    struct RewriteRequest
    {
        std::string regex;
        ExpressionFlavor flavor = ExpressionFlavor::FiniteRegex;
        std::string extra_alphabet;
        bool remove_complement = false;
        bool remove_intersection = false;
        bool remove_power = false;
        bool remove_plus = false;
        bool remove_any_symbol = false;
    };

    // Requests a language comparison between two expressions.
    struct CompareRequest
    {
        ExpressionFlavor flavor = ExpressionFlavor::FiniteRegex;
        std::string left_regex;
        std::string right_regex;
        std::string extra_alphabet;
    };

    // Complete set of requests accepted by an operation worker.
    using OperationRequest = std::variant<TranslateRequest, RewriteRequest, CompareRequest>;

    // Reports a user-facing failure without crossing the worker boundary by exception.
    struct OperationFailure
    {
        std::string message;
    };

    // Contains a translated automaton's graph layout.
    struct TranslateResult
    {
        graph::Layout graph_layout;
    };

    // Contains the parseable output of a rewrite.
    struct RewriteResult
    {
        std::string rewritten_regex;
    };

    // Stores either finite-word or omega-word language comparison data.
    using ComparisonResultData =
        std::variant<automata::analysis::RegexComparison, automata::analysis::OmegaRegexComparison>;

    // Contains a semantic language comparison.
    struct CompareResult
    {
        ComparisonResultData comparison;
    };

    // Complete set of worker outcomes, including controlled failures.
    using OperationResult =
        std::variant<OperationFailure, TranslateResult, RewriteResult, CompareResult>;
}
