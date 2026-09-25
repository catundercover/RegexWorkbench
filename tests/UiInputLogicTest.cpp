// Verifies rendering-independent macro and alphabet input behavior.
#include "app/operations/OperationValidation.hpp"
#include "ui/input/AlphabetInput.hpp"
#include "ui/input/ExpressionInputValidation.hpp"
#include "ui/input/MacroCompletionLogic.hpp"
#include "ui/input/RandomRegexLogic.hpp"

#include <iostream>
#include <string>

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

// Runs macro-completion and alphabet-validation checks.
int main()
{
    const app::operations::AlphabetParseResult alphabet =
        app::operations::parse_extra_alphabet("a b a2");
    if (!check(alphabet.valid(), "A valid alphabet was rejected.") ||
        !check(alphabet.symbols.size() == 3, "Duplicate alphabet symbols were not removed.") ||
        !check(alphabet.symbols.contains('a'), "Alphabet is missing 'a'.") ||
        !check(alphabet.symbols.contains('b'), "Alphabet is missing 'b'.") ||
        !check(alphabet.symbols.contains('2'), "Alphabet is missing '2'."))
    {
        return 1;
    }

    const app::operations::AlphabetParseResult whitespace =
        app::operations::parse_extra_alphabet(" \t\n");
    if (!check(whitespace.valid(), "Whitespace-only alphabet input was rejected.") ||
        !check(whitespace.symbols.empty(), "Whitespace became an alphabet symbol."))
    {
        return 1;
    }

    const app::operations::AlphabetParseResult invalid =
        app::operations::parse_extra_alphabet("a,#");
    if (!check(!invalid.valid(), "Punctuation was accepted as an alphabet symbol.") ||
        !check(invalid.error.has_value(), "Invalid alphabet input has no error."))
    {
        return 1;
    }

    ui::input::AlphabetInputState alphabet_input;
    alphabet_input.text = "xy";
    if (!check(
            ui::input::active_alphabet(alphabet_input).empty(),
            "A disabled additional alphabet remained active."
        ))
    {
        return 1;
    }
    alphabet_input.enabled = true;
    if (!check(
            ui::input::active_alphabet(alphabet_input) == "xy",
            "An enabled additional alphabet was not forwarded."
        ))
    {
        return 1;
    }

    const ui::input::SyntaxFeedback empty_syntax =
        ui::input::validate_expression_syntax("  ", app::operations::ExpressionFlavor::FiniteRegex);
    const ui::input::SyntaxFeedback valid_finite = ui::input::validate_expression_syntax(
        "a(b|c)*", app::operations::ExpressionFlavor::FiniteRegex
    );
    const ui::input::SyntaxFeedback invalid_finite =
        ui::input::validate_expression_syntax("a|", app::operations::ExpressionFlavor::FiniteRegex);
    const ui::input::SyntaxFeedback valid_omega = ui::input::validate_expression_syntax(
        "a*b^ω", app::operations::ExpressionFlavor::OmegaRegex
    );
    const ui::input::SyntaxFeedback omega_in_finite = ui::input::validate_expression_syntax(
        "a^ω", app::operations::ExpressionFlavor::FiniteRegex
    );
    if (!check(
            empty_syntax.validity == ui::input::SyntaxValidity::Empty,
            "A blank editor did not produce empty syntax feedback."
        ) ||
        !check(
            valid_finite.validity == ui::input::SyntaxValidity::Valid,
            "A valid finite expression did not produce valid syntax feedback."
        ) ||
        !check(
            invalid_finite.validity == ui::input::SyntaxValidity::Invalid &&
                invalid_finite.message.find("column") != std::string::npos,
            "An invalid finite expression did not preserve source-aware syntax feedback."
        ) ||
        !check(
            valid_omega.validity == ui::input::SyntaxValidity::Valid,
            "A valid omega expression did not produce valid syntax feedback."
        ) ||
        !check(
            omega_in_finite.validity == ui::input::SyntaxValidity::Invalid &&
                omega_in_finite.error_byte_offset == 1 &&
                omega_in_finite.message.find("Infinite words") != std::string::npos,
            "Finite-word omega power did not produce actionable, positioned feedback."
        ))
    {
        return 1;
    }

    const std::string finite_generated = ui::input::adapt_generated_expression(
        "a|b", app::operations::ExpressionFlavor::FiniteRegex
    );
    const std::string omega_generated =
        ui::input::adapt_generated_expression("a|b", app::operations::ExpressionFlavor::OmegaRegex);
    if (!check(
            finite_generated == "a|b", "Finite random generation unexpectedly changed domains."
        ) ||
        !check(
            omega_generated == "(a|b)^ω",
            "Omega random generation did not create an infinite repetition."
        ) ||
        !check(
            ui::input::validate_expression_syntax(
                omega_generated, app::operations::ExpressionFlavor::OmegaRegex
            )
                    .validity == ui::input::SyntaxValidity::Valid,
            "The adapted omega random expression was not valid omega syntax."
        ))
    {
        return 1;
    }

    const auto active_token = ui::input::find_active_macro_token("\\sig", 4);
    if (!active_token.has_value())
    {
        check(false, "Active macro token was not detected.");
        return 1;
    }
    const ui::input::ActiveMacroToken& token = active_token.value();
    if (!check(token.slash_position == 0, "Macro slash position is incorrect.") ||
        !check(token.prefix == "sig", "Macro prefix is incorrect."))
    {
        return 1;
    }

    const auto clamped_token = ui::input::find_active_macro_token("\\sig", 100);
    if (!clamped_token.has_value())
    {
        check(false, "Out-of-range cursor was not clamped.");
        return 1;
    }
    const ui::input::ActiveMacroToken& clamped = clamped_token.value();
    if (!check(clamped.prefix == "sig", "Clamped macro prefix is incorrect.") ||
        !check(
            !ui::input::find_active_macro_token("sigma", 5).has_value(),
            "Text without a slash was detected as a macro."
        ))
    {
        return 1;
    }

    const auto matches =
        ui::input::matching_macro_suggestions("e", app::operations::ExpressionFlavor::FiniteRegex);
    if (!check(matches.size() == 2, "Macro prefix did not produce two matches.") ||
        !check(matches[0].trigger == "epsilon", "First macro match is incorrect.") ||
        !check(matches[1].trigger == "emptyset", "Second macro match is incorrect."))
    {
        return 1;
    }

    const auto finite_omega_matches =
        ui::input::matching_macro_suggestions("om", app::operations::ExpressionFlavor::FiniteRegex);
    const auto omega_matches =
        ui::input::matching_macro_suggestions("om", app::operations::ExpressionFlavor::OmegaRegex);
    if (!check(
            finite_omega_matches.empty(), "Finite regex mode unexpectedly offered the omega macro."
        ) ||
        !check(
            omega_matches.size() == 1 && omega_matches.front().trigger == "omega" &&
                omega_matches.front().replacement == "^ω",
            "Omega regex mode did not offer the omega macro."
        ))
    {
        return 1;
    }

    std::string text = "x\\sig+y";
    std::size_t cursor_position = 5;
    if (!check(
            ui::input::replace_macro_token(text, 1, 5, "Σ", cursor_position),
            "Valid macro token was not replaced."
        ) ||
        !check(text == "xΣ+y", "Macro replacement produced incorrect text.") ||
        !check(
            cursor_position == 1 + std::string("Σ").size(),
            "Cursor was not moved after the UTF-8 replacement."
        ))
    {
        return 1;
    }

    std::string omega_text = "a\\omega";
    std::size_t omega_cursor_position = omega_text.size();
    if (!check(
            ui::input::replace_macro_token(
                omega_text, 1, omega_cursor_position, "^ω", omega_cursor_position
            ),
            "Omega macro token was not replaced."
        ) ||
        !check(omega_text == "a^ω", "Omega macro did not insert the omega-power operator.") ||
        !check(
            omega_cursor_position == omega_text.size(),
            "Cursor was not moved after the omega-power replacement."
        ))
    {
        return 1;
    }

    const std::string unchanged = text;
    if (!check(
            !ui::input::replace_macro_token(text, 0, cursor_position, "ε", cursor_position),
            "Invalid macro position was accepted."
        ) ||
        !check(text == unchanged, "Invalid macro replacement changed the text."))
    {
        return 1;
    }

    return 0;
}
