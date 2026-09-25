// Verifies operation lifecycle, timeout, cancellation, and result handling.
#include "app/operations/OperationController.hpp"

#include "support/InlineOperationRunner.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <variant>

namespace
{
    // Request type alias used by controller fixtures.
    using app::operations::OperationRequest;
    // Result type alias used by controller fixtures.
    using app::operations::OperationResult;

    // Reports a failed assertion and returns its condition.
    bool check(const bool condition, const std::string& message)
    {
        if (!condition)
        {
            std::cerr << message << '\n';
        }
        return condition;
    }

    // Starts, polls, and consumes one synchronous test operation.
    std::optional<OperationResult>
    run(app::operations::OperationController& controller, OperationRequest request)
    {
        if (!controller.start(std::move(request)))
        {
            return std::nullopt;
        }
        controller.poll();
        return controller.take_result();
    }

    // Checks that an operation failed with the exact expected diagnostic.
    bool check_failure(
        const std::optional<OperationResult>& result,
        const std::string& expected_message,
        const std::string& operation_name
    )
    {
        if (!result.has_value())
        {
            check(false, operation_name + " returned no result.");
            return false;
        }

        const auto* failure = std::get_if<app::operations::OperationFailure>(&*result);
        return check(failure != nullptr, operation_name + " unexpectedly succeeded.") &&
               check(
                   failure->message == expected_message,
                   operation_name + " returned an unexpected error: " + failure->message
               );
    }

    // Test runner that remains active until the controller cancels it.
    class NeverCompletesRunner final : public app::operations::OperationRunner
    {
    public:
        // Enters the running state without producing a result.
        bool start(OperationRequest) override
        {
            running = true;
            return true;
        }

        // Always reports that no result is available.
        std::optional<OperationResult> poll() override
        {
            return std::nullopt;
        }

        // Records cancellation and returns to the idle state.
        void cancel() override
        {
            cancelled = true;
            running = false;
        }

        // Returns the fixture's current running flag.
        [[nodiscard]] bool is_running() const noexcept override
        {
            return running;
        }

        bool running = false;
        bool cancelled = false;
    };
}

// Runs controller validation, lifecycle, timeout, and process-runner checks.
int main()
{
    auto inline_runner = std::make_unique<test_support::InlineOperationRunner>();
    app::operations::OperationController controller(std::move(inline_runner));

    if (!check_failure(
            run(controller, app::operations::TranslateRequest{}),
            "Error: Please enter a regular expression.",
            "Empty-regex validation"
        ) ||
        !check_failure(
            run(controller,
                app::operations::TranslateRequest{
                    "Σ",
                    app::operations::ExpressionFlavor::FiniteRegex,
                    app::operations::TranslateMode::Nfa,
                    ""
                }),
            "Error: Alphabet is empty",
            "Empty-sigma-alphabet validation"
        ) ||
        !check_failure(
            run(controller,
                app::operations::TranslateRequest{
                    "εΣ",
                    app::operations::ExpressionFlavor::FiniteRegex,
                    app::operations::TranslateMode::Nfa,
                    ""
                }),
            "Error: Alphabet is empty",
            "Epsilon-sigma empty-alphabet validation"
        ) ||
        !check_failure(
            run(controller,
                app::operations::TranslateRequest{
                    "a",
                    app::operations::ExpressionFlavor::FiniteRegex,
                    app::operations::TranslateMode::Nfa,
                    "@"
                }),
            "Error: Only alphanumerical symbols are valid terminals.",
            "Invalid-alphabet validation"
        ) ||
        !check_failure(
            run(controller,
                app::operations::CompareRequest{
                    app::operations::ExpressionFlavor::FiniteRegex, "a", "", ""
                }),
            "Error: Please enter both regular expressions.",
            "Missing-comparison-input validation"
        ))
    {
        return 1;
    }

    const auto epsilon_translate_result =
    run(controller,
        app::operations::TranslateRequest{
            "ε",
            app::operations::ExpressionFlavor::FiniteRegex,
            app::operations::TranslateMode::Nfa,
            ""
        });
    const auto* epsilon_translation =
        epsilon_translate_result
            ? std::get_if<app::operations::TranslateResult>(&*epsilon_translate_result)
            : nullptr;

    const auto empty_set_translate_result =
        run(controller,
            app::operations::TranslateRequest{
                "∅",
                app::operations::ExpressionFlavor::FiniteRegex,
                app::operations::TranslateMode::Nfa,
                ""
            });
    const auto* empty_set_translation =
        empty_set_translate_result
            ? std::get_if<app::operations::TranslateResult>(&*empty_set_translate_result)
            : nullptr;

    if (!check(
            epsilon_translation != nullptr,
            "Epsilon translation with an empty alphabet unexpectedly failed."
        ) ||
        !check(
            empty_set_translation != nullptr,
            "Empty-set translation with an empty alphabet unexpectedly failed."
        ))
    {
        return 1;
    }

    const auto nfa_result =
        run(controller,
            app::operations::TranslateRequest{
                "a|b",
                app::operations::ExpressionFlavor::FiniteRegex,
                app::operations::TranslateMode::Nfa,
                ""
            });
    const auto* nfa =
        nfa_result ? std::get_if<app::operations::TranslateResult>(&*nfa_result) : nullptr;
    if (!check(nfa != nullptr, "NFA translation returned the wrong result type.") ||
        !check(!nfa->graph_layout.nodes.empty(), "NFA graph has no nodes."))
    {
        return 1;
    }

    app::operations::RewriteRequest rewrite_request;
    rewrite_request.regex = "a+";
    rewrite_request.remove_plus = true;
    const auto rewrite_result = run(controller, std::move(rewrite_request));
    const auto* rewrite =
        rewrite_result ? std::get_if<app::operations::RewriteResult>(&*rewrite_result) : nullptr;
    if (!check(rewrite != nullptr, "Rewrite returned the wrong result type.") ||
        !check(!rewrite->rewritten_regex.empty(), "Rewrite returned an empty regex."))
    {
        return 1;
    }

    const auto compare_result =
        run(controller,
            app::operations::CompareRequest{
                app::operations::ExpressionFlavor::FiniteRegex, "a", "a", ""
            });
    const auto* comparison =
        compare_result ? std::get_if<app::operations::CompareResult>(&*compare_result) : nullptr;
    const auto* finite_comparison =
        comparison != nullptr
            ? std::get_if<automata::analysis::RegexComparison>(&comparison->comparison)
            : nullptr;
    if (!check(comparison != nullptr, "Comparison returned the wrong result type.") ||
        !check(finite_comparison != nullptr, "Comparison did not contain finite data.") ||
        !check(
            finite_comparison->relation == automata::analysis::LanguageRelation::Equivalent,
            "Equal regexes were not reported as equivalent."
        ))
    {
        return 1;
    }

    const auto omega_empty_translate_result =
    run(controller,
        app::operations::TranslateRequest{
            "∅",
            app::operations::ExpressionFlavor::OmegaRegex,
            app::operations::TranslateMode::Nfa,
            ""
        });
    const auto* omega_empty_translation =
    omega_empty_translate_result
        ? std::get_if<app::operations::TranslateResult>(&*omega_empty_translate_result)
        : nullptr;
    const auto* omega_empty_failure =
        omega_empty_translate_result
            ? std::get_if<app::operations::OperationFailure>(&*omega_empty_translate_result)
            : nullptr;

    if (!check(
            omega_empty_translation != nullptr,
            std::string("Omega empty-set translation with an empty alphabet unexpectedly failed") +
                (omega_empty_failure != nullptr ? ": " + omega_empty_failure->message : ".")
        ))
    {
        return 1;
    }

    if (!check_failure(
            run(controller,
                app::operations::TranslateRequest{
                    "Σ^ω",
                    app::operations::ExpressionFlavor::OmegaRegex,
                    app::operations::TranslateMode::Nfa,
                    ""
                }),
            "Error: Alphabet is empty",
            "Omega Sigma empty-alphabet validation"
        ))
    {
        return 1;
    }

    const auto omega_translate_result =
        run(controller,
            app::operations::TranslateRequest{
                "a^ω",
                app::operations::ExpressionFlavor::OmegaRegex,
                app::operations::TranslateMode::Nfa,
                "a"
            });
    const auto* omega_translation =
        omega_translate_result
            ? std::get_if<app::operations::TranslateResult>(&*omega_translate_result)
            : nullptr;
    if (!check(omega_translation != nullptr, "Omega translation returned the wrong result type.") ||
        !check(
            !omega_translation->graph_layout.nodes.empty(),
            "Omega translation returned an empty graph."
        ))
    {
        return 1;
    }

    const auto omega_prefixed_empty_result =
    run(controller,
        app::operations::TranslateRequest{
            "∅ab^ω",
            app::operations::ExpressionFlavor::OmegaRegex,
            app::operations::TranslateMode::Nfa,
            ""
        });
    const auto* omega_prefixed_empty_translation =
        omega_prefixed_empty_result
            ? std::get_if<app::operations::TranslateResult>(&*omega_prefixed_empty_result)
            : nullptr;

    if (!check(
            omega_prefixed_empty_translation != nullptr,
            "Omega expression '∅ab^ω' was treated as having an empty alphabet."
        ))
    {
        return 1;
    }

    app::operations::RewriteRequest omega_rewrite_request;
    omega_rewrite_request.regex = "(a+)^ω";
    omega_rewrite_request.flavor = app::operations::ExpressionFlavor::OmegaRegex;
    omega_rewrite_request.extra_alphabet = "a";
    omega_rewrite_request.remove_plus = true;

    const auto omega_rewrite_result = run(controller, std::move(omega_rewrite_request));
    const auto* omega_rewrite =
        omega_rewrite_result ? std::get_if<app::operations::RewriteResult>(&*omega_rewrite_result)
                             : nullptr;

    if (!check(omega_rewrite != nullptr, "Omega rewrite returned the wrong result type.") ||
        !check(
            omega_rewrite->rewritten_regex == "(aa*)^ω",
            "Omega rewrite did not preserve the omega rewrite flavor."
        ))
    {
        return 1;
    }

    auto stalled_runner = std::make_unique<NeverCompletesRunner>();
    NeverCompletesRunner* stalled = stalled_runner.get();
    app::operations::OperationController timed_controller(
        std::move(stalled_runner), std::chrono::steady_clock::duration::zero()
    );
    timed_controller.start(
        app::operations::CompareRequest{
            app::operations::ExpressionFlavor::FiniteRegex, "a", "a", ""
        }
    );
    timed_controller.poll();
    const auto timeout_result = timed_controller.take_result();
    const auto* timeout =
        timeout_result ? std::get_if<app::operations::OperationFailure>(&*timeout_result) : nullptr;
    if (!check(stalled->cancelled, "Timed-out runner was not cancelled.") ||
        !check(timeout != nullptr, "Timeout did not produce a failure result."))
    {
        return 1;
    }

#ifndef __EMSCRIPTEN__
    app::operations::OperationController process_controller;
    app::operations::RewriteRequest process_request;
    process_request.regex = "a+";
    process_request.remove_plus = true;
    if (!check(
            process_controller.start(std::move(process_request)),
            "Could not start the native operation worker."
        ))
    {
        return 1;
    }
    for (int attempt = 0; attempt < 2000 && process_controller.is_running(); ++attempt)
    {
        process_controller.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    process_controller.poll();
    const auto process_result = process_controller.take_result();
    if (!check(
            process_result.has_value() &&
                std::holds_alternative<app::operations::RewriteResult>(*process_result),
            "The native operation worker did not return a rewrite result."
        ))
    {
        return 1;
    }

    app::operations::OperationController cancelled_process;
    if (!check(
            cancelled_process.start(
                app::operations::CompareRequest{
                    app::operations::ExpressionFlavor::FiniteRegex, "(a|b)*", "a*", ""
                }
            ),
            "Could not start the native worker cancellation check."
        ))
    {
        return 1;
    }
    cancelled_process.cancel();
    if (!check(
            !cancelled_process.is_running() && !cancelled_process.take_result().has_value(),
            "Cancelling the native worker left an operation or result behind."
        ))
    {
        return 1;
    }
#endif

    return 0;
}
