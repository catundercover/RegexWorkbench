// Verifies round-trip serialization of operation requests and results.
#include "worker/protocol/OperationProtocol.hpp"

#include <iostream>
#include <string>
#include <variant>

namespace
{
    // Reports a failed assertion and returns its condition.
    bool check(const bool condition, const std::string& message)
    {
        if (!condition)
        {
            std::cerr << message << '\n';
        }
        return condition;
    }
}

// Runs request and result round-trip serialization checks.
int main()
{
    app::operations::RewriteRequest rewrite;
    rewrite.regex = "(a&b)+";
    rewrite.flavor = app::operations::ExpressionFlavor::OmegaRegex;
    rewrite.extra_alphabet = "ab";
    rewrite.remove_intersection = true;
    rewrite.remove_plus = true;

    const worker::protocol::Payload request_payload = worker::protocol::encode_request(rewrite);
    const auto decoded_request = worker::protocol::decode_request(request_payload);
    const auto* request = std::get_if<app::operations::OperationRequest>(&decoded_request);
    const auto* decoded_rewrite =
        request != nullptr ? std::get_if<app::operations::RewriteRequest>(request) : nullptr;
    if (!check(decoded_rewrite != nullptr, "Rewrite request changed type during transport.") ||
        !check(decoded_rewrite->regex == rewrite.regex, "Rewrite regex was not preserved.") ||
        !check(
            decoded_rewrite->flavor == app::operations::ExpressionFlavor::OmegaRegex,
            "Rewrite expression flavor was not preserved."
        ) ||
        !check(decoded_rewrite->extra_alphabet == "ab", "Rewrite alphabet was not preserved.") ||
        !check(
            decoded_rewrite->remove_intersection && decoded_rewrite->remove_plus,
            "Rewrite flags were not preserved."
        ))
    {
        return 1;
    }

    const auto translate_payload = worker::protocol::encode_request(
        app::operations::TranslateRequest{
            "a*",
            app::operations::ExpressionFlavor::FiniteRegex,
            app::operations::TranslateMode::Dfa,
            "ab"
        }
    );
    const auto decoded_translate_payload = worker::protocol::decode_request(translate_payload);
    const auto* translate_request =
        std::get_if<app::operations::OperationRequest>(&decoded_translate_payload);
    const auto* translate = translate_request != nullptr
                                ? std::get_if<app::operations::TranslateRequest>(translate_request)
                                : nullptr;
    if (!check(
            translate != nullptr &&
                translate->flavor == app::operations::ExpressionFlavor::FiniteRegex &&
                translate->mode == app::operations::TranslateMode::Dfa &&
                translate->extra_alphabet == "ab",
            "Translation request was not preserved."
        ))
    {
        return 1;
    }

    const auto omega_translate_payload = worker::protocol::encode_request(
        app::operations::TranslateRequest{
            "a^ω",
            app::operations::ExpressionFlavor::OmegaRegex,
            app::operations::TranslateMode::Nfa,
            "a"
        }
    );
    const auto decoded_omega_translate_payload =
        worker::protocol::decode_request(omega_translate_payload);
    const auto* omega_translate_request =
        std::get_if<app::operations::OperationRequest>(&decoded_omega_translate_payload);
    const auto* omega_translate =
        omega_translate_request != nullptr
            ? std::get_if<app::operations::TranslateRequest>(omega_translate_request)
            : nullptr;
    if (!check(
            omega_translate != nullptr && omega_translate->regex == "a^ω" &&
                omega_translate->flavor == app::operations::ExpressionFlavor::OmegaRegex,
            "Omega translation request was not preserved."
        ))
    {
        return 1;
    }

    graph::Layout layout;
    layout.bounds = {{-1.0f, -2.0f}, {40.0f, 30.0f}};
    layout.nodes.push_back(
        graph::Node{"q0", "0", {10.0f, 12.0f}, 24.0f, true, graph::NodeRole::State}
    );
    graph::Edge edge;
    edge.source = "q0";
    edge.target = "q0";
    edge.label = "a";
    edge.spline.push_back({{1.0f, 2.0f}, {3.0f, 4.0f}, {5.0f, 6.0f}, {7.0f, 8.0f}});
    edge.arrow_tip = graph::Point{9.0f, 10.0f};
    edge.label_position = graph::Point{11.0f, 12.0f};
    edge.label_alignment = graph::LabelAlignment::Right;
    layout.edges.push_back(edge);

    const worker::protocol::Payload result_payload =
        worker::protocol::encode_result(app::operations::TranslateResult{std::move(layout)});
    const auto decoded_result = worker::protocol::decode_result(result_payload);
    const auto* result = std::get_if<app::operations::OperationResult>(&decoded_result);
    const auto* translation =
        result != nullptr ? std::get_if<app::operations::TranslateResult>(result) : nullptr;
    if (!check(translation != nullptr, "Translation result changed type during transport.") ||
        !check(translation->graph_layout.nodes.size() == 1, "Graph nodes were not preserved.") ||
        !check(translation->graph_layout.edges.size() == 1, "Graph edges were not preserved.") ||
        !check(
            translation->graph_layout.edges.front().label_alignment == graph::LabelAlignment::Right,
            "Graph edge metadata was not preserved."
        ))
    {
        return 1;
    }

    automata::analysis::RegexComparison comparison;
    comparison.relation = automata::analysis::LanguageRelation::LeftSubsetRight;
    comparison.left_only_witness = "a";
    comparison.intersection_witness = "ε";
    comparison.right_universal = true;
    const auto comparison_payload =
        worker::protocol::encode_result(app::operations::CompareResult{comparison});
    const auto decoded_comparison_payload = worker::protocol::decode_result(comparison_payload);
    const auto* comparison_result =
        std::get_if<app::operations::OperationResult>(&decoded_comparison_payload);
    const auto* decoded_comparison =
        comparison_result != nullptr
            ? std::get_if<app::operations::CompareResult>(comparison_result)
            : nullptr;
    const auto* decoded_finite_comparison =
        decoded_comparison != nullptr
            ? std::get_if<automata::analysis::RegexComparison>(&decoded_comparison->comparison)
            : nullptr;
    if (!check(
            decoded_finite_comparison != nullptr &&
                decoded_finite_comparison->relation == comparison.relation &&
                decoded_finite_comparison->left_only_witness == "a" &&
                decoded_finite_comparison->intersection_witness == "ε" &&
                decoded_finite_comparison->right_universal,
            "Finite comparison result was not preserved."
        ))
    {
        return 1;
    }

    automata::analysis::OmegaRegexComparison omega_comparison;
    omega_comparison.relation = automata::analysis::LanguageRelation::Equivalent;
    omega_comparison.intersection_witness = automata::analysis::OmegaWitness{"a", "b"};
    omega_comparison.left_universal = true;

    const auto omega_comparison_payload =
        worker::protocol::encode_result(app::operations::CompareResult{omega_comparison});
    const auto decoded_omega_comparison_payload =
        worker::protocol::decode_result(omega_comparison_payload);
    const auto* omega_comparison_result =
        std::get_if<app::operations::OperationResult>(&decoded_omega_comparison_payload);
    const auto* decoded_omega_result =
        omega_comparison_result != nullptr
            ? std::get_if<app::operations::CompareResult>(omega_comparison_result)
            : nullptr;
    const auto* decoded_omega_comparison =
        decoded_omega_result != nullptr ? std::get_if<automata::analysis::OmegaRegexComparison>(
                                              &decoded_omega_result->comparison
                                          )
                                        : nullptr;

    if (!check(
            decoded_omega_comparison != nullptr &&
                decoded_omega_comparison->relation == omega_comparison.relation &&
                decoded_omega_comparison->intersection_witness.has_value() &&
                decoded_omega_comparison->intersection_witness->prefix == "a" &&
                decoded_omega_comparison->intersection_witness->cycle == "b" &&
                decoded_omega_comparison->left_universal,
            "Omega comparison result was not preserved."
        ))
    {
        return 1;
    }

    worker::protocol::Payload trailing_data = result_payload;
    trailing_data.push_back(0);
    if (!check(
            std::holds_alternative<worker::protocol::ProtocolError>(
                worker::protocol::decode_result(trailing_data)
            ),
            "Trailing protocol data was accepted."
        ) ||
        !check(
            std::holds_alternative<worker::protocol::ProtocolError>(
                worker::protocol::decode_result({})
            ),
            "An empty protocol payload was accepted."
        ))
    {
        return 1;
    }

    return 0;
}
