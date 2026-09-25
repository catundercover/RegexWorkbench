// Implements language classification and shortest-region witnesses.
#include "automata/analysis/RegexComparison.hpp"

#include "automata/conversion/MataAdapter.hpp"
#include "automata/conversion/RegexAlphabet.hpp"
#include "automata/conversion/RegexToNfa.hpp"

namespace automata::analysis
{
    RegexComparison compare(
        const regex::Expression& left,
        const regex::Expression& right,
        const Alphabet& extra_alphabet
    )
    {
        Alphabet alphabet = conversion::alphabet_of(left, extra_alphabet);
        const Alphabet right_alphabet = conversion::alphabet_of(right);
        alphabet.insert(right_alphabet.begin(), right_alphabet.end());

        const auto mata_alphabet = conversion::detail::to_mata_alphabet(alphabet);
        const mata::nfa::Nfa left_dfa = conversion::detail::minimize(
            conversion::detail::to_mata(conversion::to_nfa(left, alphabet))
        );
        const mata::nfa::Nfa right_dfa = conversion::detail::minimize(
            conversion::detail::to_mata(conversion::to_nfa(right, alphabet))
        );

        const mata::nfa::Nfa left_complement = mata::nfa::complement(left_dfa, mata_alphabet);
        const mata::nfa::Nfa right_complement = mata::nfa::complement(right_dfa, mata_alphabet);

        // These four products partition every word over the common alphabet.
        const mata::nfa::Nfa left_only = mata::nfa::intersection(left_dfa, right_complement);
        const mata::nfa::Nfa right_only = mata::nfa::intersection(right_dfa, left_complement);
        const mata::nfa::Nfa intersection = mata::nfa::intersection(left_dfa, right_dfa);
        const mata::nfa::Nfa neither = mata::nfa::intersection(left_complement, right_complement);

        RegexComparison result;
        result.left_only_witness = conversion::detail::shortest_word(left_only);
        result.right_only_witness = conversion::detail::shortest_word(right_only);
        result.intersection_witness = conversion::detail::shortest_word(intersection);
        result.neither_witness = conversion::detail::shortest_word(neither);
        result.left_empty = left_dfa.is_lang_empty();
        result.right_empty = right_dfa.is_lang_empty();
        result.left_universal = left_complement.is_lang_empty();
        result.right_universal = right_complement.is_lang_empty();

        const bool left_only_empty = !result.left_only_witness;
        const bool right_only_empty = !result.right_only_witness;
        const bool intersection_empty = !result.intersection_witness;
        const bool neither_empty = !result.neither_witness;

        // Relation precedence handles special cases before the general overlap classification.
        if (left_only_empty && right_only_empty)
        {
            result.relation = LanguageRelation::Equivalent;
        }
        else if (intersection_empty && neither_empty)
        {
            result.relation = LanguageRelation::Complement;
        }
        else if (left_only_empty)
        {
            result.relation = LanguageRelation::LeftSubsetRight;
        }
        else if (right_only_empty)
        {
            result.relation = LanguageRelation::RightSubsetLeft;
        }
        else if (intersection_empty)
        {
            result.relation = LanguageRelation::Disjoint;
        }
        else
        {
            result.relation = LanguageRelation::Overlap;
        }

        return result;
    }
}
