// Implements high-level NFA reduction and DFA minimization pipelines.
#include "automata/translation/RegexTranslation.hpp"

#include "automata/conversion/MataAdapter.hpp"
#include "automata/conversion/RegexAlphabet.hpp"
#include "automata/conversion/RegexToNfa.hpp"

namespace automata::translation
{
    namespace
    {
        // Builds the canonical complete one-state DFA for the empty language.
        Nfa empty_language_dfa(const Alphabet& alphabet)
        {
            Nfa result;
            const StateId sink = result.add_state();
            result.start = sink;
            for (Symbol symbol : alphabet)
            {
                result.add_transition(sink, symbol, sink);
            }
            return result;
        }
    }

    Nfa minimal_dfa(const regex::Expression& expression, const Alphabet& extra_alphabet)
    {
        const Alphabet alphabet = conversion::alphabet_of(expression, extra_alphabet);
        const mata::nfa::Nfa minimized = conversion::detail::minimize(
            conversion::detail::to_mata(conversion::to_nfa(expression, alphabet))
        );

        // MATA represents the empty language without states; the public model requires a start.
        if (minimized.is_lang_empty())
        {
            return empty_language_dfa(alphabet);
        }

        Nfa result = conversion::detail::from_mata(minimized);
        result.alphabet = alphabet;
        return result;
    }

    Nfa epsilon_free_nfa(const regex::Expression& expression, const Alphabet& extra_alphabet)
    {
        const Alphabet alphabet = conversion::alphabet_of(expression, extra_alphabet);
        mata::nfa::Nfa result =
            mata::nfa::remove_epsilon(
                conversion::detail::to_mata(conversion::to_nfa(expression, alphabet))
            )
                .trim();

        result =
            mata::nfa::reduce(result, nullptr, mata::nfa::ParameterMap{{"algorithm", "simulation"}})
                .trim();

        Nfa converted = conversion::detail::from_mata(result);
        converted.alphabet = alphabet;
        return converted;
    }
}
