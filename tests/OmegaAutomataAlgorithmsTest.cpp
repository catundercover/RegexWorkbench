// Verifies omega-regex translation, Büchi rendering, and Spot-backed comparison.
#include "app/operations/OperationExecutor.hpp"
#include "app/operations/OperationModels.hpp"
#include "automata/analysis/OmegaRegexComparison.hpp"
#include "automata/conversion/OmegaRegexToSpot.hpp"
#include "automata/conversion/SpotAlphabetEncoding.hpp"
#include "automata/conversion/SpotToOmegaRegex.hpp"
#include "automata/dot/BuchiDotExporter.hpp"
#include "automata/translation/OmegaRegexTranslation.hpp"
#include "regex/parsing/OmegaRegexParser.hpp"
#include "support/TestSupport.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <spot/twaalgos/contains.hh>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

namespace
{
    // Parses a required omega expression used by an automata test.
    regex::omega::Expression parse_omega(const std::string_view text)
    {
        regex::parsing::omega::ParseResult result = regex::parsing::omega::parse(text);
        if (!result.expression.has_value() || result.error.has_value())
        {
            throw std::runtime_error("Test omega expression did not parse: " + std::string(text));
        }
        return std::move(result.expression).value();
    }

    // Compares two omega expressions and checks the expected relation.
    bool has_relation(
        const std::string_view left,
        const std::string_view right,
        const automata::analysis::LanguageRelation expected,
        const automata::Alphabet& alphabet = {'a', 'b', 'c'}
    )
    {
        const automata::analysis::OmegaRegexComparison comparison =
            automata::analysis::compare(parse_omega(left), parse_omega(right), alphabet);

        return test_support::check(
            comparison.relation == expected,
            "Omega relation was misclassified for " + std::string(left) + " and " +
                std::string(right) + "."
        );
    }

    // Returns whether an omega witness has a non-empty ultimately periodic cycle.
    bool valid_witness(const std::optional<automata::analysis::OmegaWitness>& witness)
    {
        return witness.has_value() && !witness->cycle.empty();
    }

    // Returns whether every character in a witness belongs to the active alphabet.
    bool witness_uses_only(
        const std::optional<automata::analysis::OmegaWitness>& witness,
        const automata::Alphabet& alphabet
    )
    {
        if (!witness.has_value())
        {
            return true;
        }

        const auto belongs = [&alphabet](const char symbol) { return alphabet.contains(symbol); };
        return std::ranges::all_of(witness->prefix, belongs) &&
               std::ranges::all_of(witness->cycle, belongs);
    }

    // Rebuilds the displayed graph, including state acceptance, for an exact
    // language check against the independent raw omega-regex construction.
    bool display_preserves_language(
        const automata::BuchiAutomaton& display, const regex::omega::Expression& expression
    )
    {
        const auto reference = automata::conversion::to_spot_twa_raw(expression, display.alphabet);
        const auto actual = spot::make_twa_graph(reference->get_dict());
        automata::conversion::spot_encoding::register_alphabet(actual, display.alphabet);
        actual->set_buchi();
        actual->new_states(static_cast<unsigned>(display.transitions.size()));
        actual->set_init_state(static_cast<unsigned>(display.start));
        for (automata::StateId state = 0; state < display.transitions.size(); ++state)
        {
            for (const auto& edge : display.transitions[state])
            {
                bdd condition = bddfalse;
                for (const auto symbol : edge.symbols)
                {
                    condition |= automata::conversion::spot_encoding::symbol_condition(
                        actual, display.alphabet, symbol
                    );
                }
                actual->new_edge(
                    static_cast<unsigned>(state),
                    static_cast<unsigned>(edge.target),
                    condition,
                    display.finals.contains(state) ? spot::acc_cond::mark_t{0}
                                                   : spot::acc_cond::mark_t{}
                );
            }
        }
        actual->prop_state_acc(true);
        return spot::are_equivalent(reference, actual);
    }

    bool deterministic(const automata::BuchiAutomaton& automaton)
    {
        for (const auto& outgoing : automaton.transitions)
        {
            automata::Alphabet seen;
            for (const auto& edge : outgoing)
            {
                for (const auto symbol : edge.symbols)
                {
                    if (!seen.insert(symbol).second)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool translation_regressions()
    {
        struct Fixture
        {
            std::string_view expression;
            std::size_t minimal_states;
        };
        // Sizes are for partial, state-based DBAs. In particular GF(a) needs
        // two states, although its transition-based DBA needs only one.
        const std::array fixtures{
            Fixture{"a^ω", 1},
            Fixture{"(ab)^ω", 2},
            Fixture{"(a|b)^ω", 1},
            Fixture{"∅", 1},
            Fixture{"a^ω&b^ω", 1},
            Fixture{"!∅", 1},
            Fixture{"a*b^ω", 2},
            Fixture{"a^ω|b^ω", 3},
            Fixture{"(ab)^ω|(ba)^ω", 3},
            Fixture{"a*(ab)^ω", 4},
            Fixture{"(a|b)*a(a|b)^ω", 2},
            Fixture{"(b*a)^ω", 2},
            Fixture{"((a|b)*aa)^ω", 3},
            Fixture{"!((a|b)*a^ω)", 2},
            Fixture{"(b*a)^ω&(a*b)^ω", 3}
        };
        for (const auto& [text, states] : fixtures)
        {
            const auto expression = parse_omega(text);
            const auto nba = automata::translation::buchi_automaton(expression, {'a', 'b'});
            const auto dba = automata::translation::minimal_dba(expression, {'a', 'b'});
            if (!test_support::check(
                    nba.is_valid() && display_preserves_language(nba, expression),
                    "NBA changed the language of " + std::string(text)
                ) ||
                !test_support::check(dba.has_value(), "DBA was not found for " + std::string(text)))
            {
                return false;
            }
            if (!test_support::check(
                    dba->is_valid() && deterministic(*dba) &&
                        display_preserves_language(*dba, expression),
                    "DBA display is invalid, nondeterministic, or inequivalent for " +
                        std::string(text)
                ) ||
                !test_support::check(
                    dba->transitions.size() == states,
                    "DBA is not state-minimal for " + std::string(text)
                ))
            {
                return false;
            }
        }
        const std::array small_nbas{
            Fixture{"a^ω", 1},
            Fixture{"(ab)^ω", 2},
            Fixture{"(a|b)^ω", 1},
            // The previous pipeline produced 5, 3, and 3 states respectively.
            Fixture{"(ab)^ω|(ba)^ω", 3},
            Fixture{"(a|b)*ab^ω", 2},
            Fixture{"(a|b)*a(a|b)^ω", 2}
        };
        for (const auto& [text, expected] : small_nbas)
        {
            const auto expression = parse_omega(text);
            const auto nba = automata::translation::buchi_automaton(expression, {'a', 'b'});
            if (!test_support::check(
                    nba.transitions.size() == expected &&
                        display_preserves_language(nba, expression),
                    "Simple NBA retained redundant states for " + std::string(text)
                ))
            {
                return false;
            }
        }
        for (const auto text : {"(a|b)*a^ω", "!((b*a)^ω)", "(a|b)*(a^ω|b^ω)"})
        {
            if (!test_support::check(
                    !automata::translation::minimal_dba(parse_omega(text), {'a', 'b'}),
                    "A non-DBA language was reported as deterministic: " + std::string(text)
                ))
            {
                return false;
            }
        }
        // Three/five symbols exercise the many-to-one Boolean letter encoding
        // after SAT synthesis, as well as extra alphabet symbols in complements.
        for (const automata::Alphabet& alphabet :
             {automata::Alphabet{'a', 'b', 'c'}, automata::Alphabet{'a', 'b', 'c', 'd', 'e'}})
        {
            for (const auto text : {"((a|b)*c)^ω", "!((a|b|c)*a^ω)", "(a|b|c)^ω"})
            {
                const auto expression = parse_omega(text);
                const auto dba = automata::translation::minimal_dba(expression, alphabet);
                if (!test_support::check(
                        dba && deterministic(*dba) && display_preserves_language(*dba, expression),
                        "DBA mishandled a non-power-of-two alphabet for " + std::string(text)
                    ))
                {
                    return false;
                }
            }
        }
        return true;
    }
}

// Runs deterministic omega automata algorithm and operation-dispatch checks.
int main()
{
    using enum automata::analysis::LanguageRelation;

    if (!translation_regressions())
    {
        return 1;
    }

    const regex::omega::Expression simple = parse_omega("a^ω");
    const automata::BuchiAutomaton buchi =
        automata::translation::buchi_automaton(simple, {'a', 'b'});

    if (!test_support::check(buchi.is_valid(), "A translated Büchi automaton is invalid.") ||
        !test_support::check(!buchi.transitions.empty(), "The Büchi automaton has no states.") ||
        !test_support::check(
            !buchi.finals.empty(), "The state-based Büchi automaton has no finals."
        ))
    {
        return 1;
    }

    const std::string dot = automata::dot::to_dot(buchi);
    if (!test_support::check(
            dot.find("digraph BuchiAutomaton") != std::string::npos,
            "Büchi DOT output has the wrong graph header."
        ) ||
        !test_support::check(
            dot.find("shape=doublecircle") != std::string::npos,
            "Büchi DOT output does not mark accepting states."
        ))
    {
        return 1;
    }

    const automata::BuchiAutomaton universal_buchi =
        automata::translation::buchi_automaton(parse_omega("(a|b)^ω"), {'a', 'b'});
    const std::string universal_dot = automata::dot::to_dot(universal_buchi);
    if (!test_support::check(
            universal_dot.find("Σ") != std::string::npos,
            "A transition over the complete alphabet was not rendered as Sigma."
        ) ||
        !test_support::check(
            universal_dot.find("__regexthesis_letter_bit_") == std::string::npos,
            "An internal Spot proposition leaked into Büchi DOT output."
        ))
    {
        return 1;
    }

    const automata::BuchiAutomaton overlapping_edges{
        0, {0}, {{{{'a', 'b'}, 0}, {{'b', 'c'}, 0}}}, {'a', 'b', 'c'}
    };
    const std::string overlapping_dot = automata::dot::to_dot(overlapping_edges);
    if (!test_support::check(
            overlapping_dot.find("label=\"Σ\"") != std::string::npos,
            "Overlapping transitions to the same state were not merged as a symbol set."
        ))
    {
        return 1;
    }

    if (!has_relation("a^ω", "a^ω", Equivalent) || !has_relation("a^ω", "b^ω", Disjoint) ||
        !has_relation("a^ω", "!a^ω", Complement, {'a', 'b'}) ||
        !has_relation("(ε|a)^ω", "a^ω", Equivalent, {'a'}) ||
        !has_relation("εa^ω", "a^ω", Equivalent, {'a'}) ||
        !has_relation("a^ω", "(a|b)^ω", LeftSubsetRight, {'a', 'b'}) ||
        !has_relation("(a|b)^ω", "a^ω", RightSubsetLeft, {'a', 'b'}) ||
        !has_relation("(a|b)^ω", "(b|c)^ω", Overlap, {'a', 'b', 'c'}) ||
        !has_relation("∅", "a^ω", LeftSubsetRight, {'a', 'b'}) ||
        !has_relation("∅", "∅", Equivalent, {'a'}))
    {
        return 1;
    }

    const automata::analysis::OmegaRegexComparison universal =
        automata::analysis::compare(parse_omega("!∅"), parse_omega("(a|b|c)^ω"), {'a', 'b', 'c'});
    const regex::omega::Expression empty_intersection =
        std::make_shared<const regex::omega::Intersection>(regex::omega::Intersection{});
    const automata::analysis::OmegaRegexComparison empty_intersection_comparison =
        automata::analysis::compare(empty_intersection, parse_omega("!∅"), {'a'});
    if (!test_support::check(
            universal.relation == Equivalent && universal.left_universal &&
                universal.right_universal,
            "Complement was not interpreted relative to the character alphabet."
        ) ||
        !test_support::check(
            empty_intersection_comparison.relation == Equivalent,
            "A zero-operand omega intersection was not interpreted as the universal language."
        ) ||
        !has_relation("(a^ω&!b^ω)|b^ω", "a^ω|b^ω", Equivalent, {'a', 'b'}))
    {
        return 1;
    }

    const automata::analysis::OmegaRegexComparison overlap = automata::analysis::compare(
        parse_omega("(a|b)^ω"), parse_omega("(b|c)^ω"), {'a', 'b', 'c'}
    );

    if (!test_support::check(
            valid_witness(overlap.left_only_witness),
            "Omega overlap has no left-only ultimately periodic witness."
        ) ||
        !test_support::check(
            valid_witness(overlap.right_only_witness),
            "Omega overlap has no right-only ultimately periodic witness."
        ) ||
        !test_support::check(
            valid_witness(overlap.intersection_witness),
            "Omega overlap has no shared ultimately periodic witness."
        ) ||
        !test_support::check(
            witness_uses_only(overlap.left_only_witness, {'a', 'b', 'c'}) &&
                witness_uses_only(overlap.right_only_witness, {'a', 'b', 'c'}) &&
                witness_uses_only(overlap.intersection_witness, {'a', 'b', 'c'}) &&
                witness_uses_only(overlap.neither_witness, {'a', 'b', 'c'}),
            "An omega witness contained an internal BDD valuation instead of a character."
        ))
    {
        return 1;
    }

    const automata::Alphabet loop_alphabet{'a', 'b', 'c'};
    const spot::twa_graph_ptr endpoint_loops = spot::make_twa_graph(spot::make_bdd_dict());
    automata::conversion::spot_encoding::register_alphabet(endpoint_loops, loop_alphabet);
    endpoint_loops->set_buchi();
    endpoint_loops->new_states(2);
    endpoint_loops->set_init_state(0);
    endpoint_loops->new_edge(
        0,
        0,
        automata::conversion::spot_encoding::symbol_condition(endpoint_loops, loop_alphabet, 'a')
    );
    endpoint_loops->new_edge(
        0,
        1,
        automata::conversion::spot_encoding::symbol_condition(endpoint_loops, loop_alphabet, 'b')
    );
    endpoint_loops->new_edge(
        1,
        1,
        automata::conversion::spot_encoding::symbol_condition(endpoint_loops, loop_alphabet, 'c'),
        {0}
    );

    const regex::omega::Expression loops_as_regex =
        automata::conversion::to_omega_regex_ast(endpoint_loops, loop_alphabet);
    const automata::analysis::OmegaRegexComparison loops_comparison =
        automata::analysis::compare(loops_as_regex, parse_omega("a*bc^ω"), loop_alphabet);
    if (!test_support::check(
            loops_comparison.relation == Equivalent,
            "Spot-to-regex conversion lost a start-state or accepting-state loop."
        ))
    {
        return 1;
    }

    const app::operations::OperationResult omega_empty_operation = app::operations::execute(
        app::operations::TranslateRequest{
            "∅",
            app::operations::ExpressionFlavor::OmegaRegex,
            app::operations::TranslateMode::Nfa,
            ""
        }
    );

    const app::operations::OperationResult omega_prefixed_empty_operation =
        app::operations::execute(
            app::operations::TranslateRequest{
                "∅ab^ω",
                app::operations::ExpressionFlavor::OmegaRegex,
                app::operations::TranslateMode::Nfa,
                ""
            }
        );

    const app::operations::OperationResult omega_sigma_without_alphabet = app::operations::execute(
        app::operations::TranslateRequest{
            "Σ^ω",
            app::operations::ExpressionFlavor::OmegaRegex,
            app::operations::TranslateMode::Nfa,
            ""
        }
    );

    const auto* omega_empty_translated =
    std::get_if<app::operations::TranslateResult>(&omega_empty_operation);
    const auto* omega_empty_failure =
        std::get_if<app::operations::OperationFailure>(&omega_empty_operation);
    const auto* omega_prefixed_empty_translated =
        std::get_if<app::operations::TranslateResult>(&omega_prefixed_empty_operation);
    const auto* omega_prefixed_empty_failure =
        std::get_if<app::operations::OperationFailure>(&omega_prefixed_empty_operation);
    const auto* omega_sigma_failure =
        std::get_if<app::operations::OperationFailure>(&omega_sigma_without_alphabet);

    if (!test_support::check(
            omega_empty_translated != nullptr,
            std::string("Omega empty set unexpectedly failed with an empty active alphabet") +
                (omega_empty_failure != nullptr ? ": " + omega_empty_failure->message : ".")
        ) ||
        !test_support::check(
            omega_prefixed_empty_translated != nullptr,
            std::string("Omega expression '∅ab^ω' was treated as alphabet-empty") +
                (omega_prefixed_empty_failure != nullptr
                     ? ": " + omega_prefixed_empty_failure->message
                     : ".")
        ) ||
        !test_support::check(
            omega_sigma_failure != nullptr &&
                omega_sigma_failure->message == "Error: Alphabet is empty",
            "Omega Sigma without an active alphabet did not fail with the expected error."
        ))
    {
        return 1;
    }

    const app::operations::OperationResult omega_sigma_with_erased_terminal_source =
    app::operations::execute(
        app::operations::TranslateRequest{
            "∅ah^ω|Σ^ω",
            app::operations::ExpressionFlavor::OmegaRegex,
            app::operations::TranslateMode::Nfa,
            ""
        }
    );

    const auto* omega_sigma_with_erased_terminal_source_translated =
        std::get_if<app::operations::TranslateResult>(
            &omega_sigma_with_erased_terminal_source
        );
    const auto* omega_sigma_with_erased_terminal_source_failure =
        std::get_if<app::operations::OperationFailure>(
            &omega_sigma_with_erased_terminal_source
        );

    if (!test_support::check(
            omega_sigma_with_erased_terminal_source_translated != nullptr,
            std::string("Omega expression '∅ah^ω|Σ^ω' lost its pre-normalization alphabet") +
                (omega_sigma_with_erased_terminal_source_failure != nullptr
                     ? ": " + omega_sigma_with_erased_terminal_source_failure->message
                     : ".")
        ))
    {
        return 1;
    }

    const app::operations::OperationResult translate_result = app::operations::execute(
        app::operations::TranslateRequest{
            "a^ω",
            app::operations::ExpressionFlavor::OmegaRegex,
            app::operations::TranslateMode::Nfa,
            "ab"
        }
    );

    const auto* translated = std::get_if<app::operations::TranslateResult>(&translate_result);
    if (!test_support::check(
            translated != nullptr, "Omega translate operation did not return a translation result."
        ) ||
        !test_support::check(
            !translated->graph_layout.nodes.empty(),
            "Omega translate operation produced an empty graph layout."
        ))
    {
        return 1;
    }

    const app::operations::OperationResult compare_result = app::operations::execute(
        app::operations::CompareRequest{
            app::operations::ExpressionFlavor::OmegaRegex, "a^ω", "a^ω", "a"
        }
    );

    const auto* compared = std::get_if<app::operations::CompareResult>(&compare_result);
    const auto* omega_comparison =
        compared != nullptr
            ? std::get_if<automata::analysis::OmegaRegexComparison>(&compared->comparison)
            : nullptr;

    if (!test_support::check(
            omega_comparison != nullptr,
            "Omega compare operation did not return an omega comparison result."
        ) ||
        !test_support::check(
            omega_comparison->relation == Equivalent,
            "Omega compare operation misclassified equal expressions."
        ))
    {
        return 1;
    }

    app::operations::RewriteRequest omega_rewrite_request;
    omega_rewrite_request.regex = "(a+)^ω";
    omega_rewrite_request.flavor = app::operations::ExpressionFlavor::OmegaRegex;
    omega_rewrite_request.extra_alphabet = "a";
    omega_rewrite_request.remove_plus = true;

    const app::operations::OperationResult omega_rewrite_result =
        app::operations::execute(omega_rewrite_request);
    const auto* omega_rewrite = std::get_if<app::operations::RewriteResult>(&omega_rewrite_result);

    if (!test_support::check(
            omega_rewrite != nullptr, "Omega rewrite operation did not return a rewrite result."
        ) ||
        !test_support::check(
            omega_rewrite->rewritten_regex == "(aa*)^ω",
            "Omega rewrite did not expand plus inside an omega-power operand; got " +
                (omega_rewrite != nullptr ? omega_rewrite->rewritten_regex : "<no result>") + "."
        ))
    {
        return 1;
    }

    app::operations::RewriteRequest invalid_omega_rewrite_request;
    invalid_omega_rewrite_request.regex = "a+";
    invalid_omega_rewrite_request.flavor = app::operations::ExpressionFlavor::OmegaRegex;
    invalid_omega_rewrite_request.extra_alphabet = "a";

    const app::operations::OperationResult invalid_omega_rewrite =
        app::operations::execute(invalid_omega_rewrite_request);

    if (!test_support::check(
            std::holds_alternative<app::operations::OperationFailure>(invalid_omega_rewrite),
            "Finite-only expression unexpectedly rewrote in omega mode."
        ))
    {
        return 1;
    }

    const app::operations::OperationResult invalid_omega = app::operations::execute(
        app::operations::TranslateRequest{
            "a",
            app::operations::ExpressionFlavor::OmegaRegex,
            app::operations::TranslateMode::Nfa,
            "a"
        }
    );

    const auto* failure = std::get_if<app::operations::OperationFailure>(&invalid_omega);
    if (!test_support::check(
            failure != nullptr, "Finite-only expression unexpectedly translated in omega mode."
        ))
    {
        return 1;
    }

    return 0;
}
