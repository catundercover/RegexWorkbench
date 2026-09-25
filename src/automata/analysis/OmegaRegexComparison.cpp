// Implements omega-language comparison using Spot automata operations.
#include "automata/analysis/OmegaRegexComparison.hpp"

#include "automata/conversion/OmegaRegexAlphabet.hpp"
#include "automata/conversion/OmegaRegexToSpot.hpp"
#include "automata/conversion/SpotAlphabetEncoding.hpp"

#include <optional>
#include <spot/twaalgos/complement.hh>
#include <spot/twaalgos/postproc.hh>
#include <spot/twaalgos/product.hh>
#include <spot/twaalgos/word.hh>
#include <stdexcept>
#include <string>
#include <vector>

namespace automata::analysis
{
    namespace
    {
        spot::twa_graph_ptr postprocess_for_comparison(const spot::twa_graph_ptr& automaton)
        {
            if (!automaton)
            {
                throw std::invalid_argument("Cannot compare a null Spot automaton");
            }

            spot::postprocessor postprocessor;
            postprocessor.set_type(spot::postprocessor::Generic);
            postprocessor.set_pref(spot::postprocessor::Small);
            postprocessor.set_level(spot::postprocessor::Medium);
            return postprocessor.run(automaton);
        }

        std::size_t transition_count(const spot::twa_graph_ptr& automaton)
        {
            std::size_t count = 0;
            for (unsigned state = 0; state < automaton->num_states(); ++state)
            {
                for ([[maybe_unused]] const auto& edge : automaton->out(state))
                {
                    ++count;
                }
            }
            return count;
        }

        bool is_large_for_comparison(const spot::twa_graph_ptr& automaton)
        {
            return automaton->num_states() >= 64 || transition_count(automaton) >= 512;
        }

        spot::twa_graph_ptr maybe_postprocess_for_comparison(const spot::twa_graph_ptr& automaton)
        {
            return is_large_for_comparison(automaton) ? postprocess_for_comparison(automaton)
                                                      : automaton;
        }

        spot::twa_graph_ptr
        intersect(const spot::twa_graph_ptr& left, const spot::twa_graph_ptr& right)
        {
            return postprocess_for_comparison(
                maybe_postprocess_for_comparison(spot::product(left, right))
            );
        }

        spot::twa_graph_ptr complement(const spot::twa_graph_ptr& automaton)
        {
            const spot::twa_graph_ptr prepared = maybe_postprocess_for_comparison(automaton);
            return postprocess_for_comparison(spot::complement(prepared));
        }

        std::string word_part_to_string(
            const spot::twa_word::seq_t& part,
            const spot::twa_graph_ptr& automaton,
            const Alphabet& alphabet
        )
        {
            std::string output;

            for (const bdd& letter : part)
            {
                const std::vector<Symbol> symbols =
                    conversion::spot_encoding::satisfying_symbols(automaton, alphabet, letter);
                if (symbols.empty())
                {
                    throw std::runtime_error("Spot produced a witness outside the active alphabet");
                }
                output += symbols.front();
            }

            return output;
        }

        std::optional<OmegaWitness>
        witness(const spot::twa_graph_ptr& automaton, const Alphabet& alphabet)
        {
            const spot::twa_word_ptr word = automaton->accepting_word();
            if (!word)
            {
                return std::nullopt;
            }

            OmegaWitness output;
            output.prefix = word_part_to_string(word->prefix, automaton, alphabet);
            output.cycle = word_part_to_string(word->cycle, automaton, alphabet);

            if (output.cycle.empty())
            {
                throw std::runtime_error("Spot produced an empty omega-witness cycle");
            }

            return output;
        }

        LanguageRelation classify(
            bool left_only_empty, bool right_only_empty, bool intersection_empty, bool neither_empty
        )
        {
            if (left_only_empty && right_only_empty)
            {
                return LanguageRelation::Equivalent;
            }

            if (intersection_empty && neither_empty)
            {
                return LanguageRelation::Complement;
            }

            if (left_only_empty)
            {
                return LanguageRelation::LeftSubsetRight;
            }

            if (right_only_empty)
            {
                return LanguageRelation::RightSubsetLeft;
            }

            if (intersection_empty)
            {
                return LanguageRelation::Disjoint;
            }

            return LanguageRelation::Overlap;
        }
    }

    OmegaRegexComparison compare(
        const regex::omega::Expression& left,
        const regex::omega::Expression& right,
        const Alphabet& extra_alphabet
    )
    {
        Alphabet alphabet = conversion::alphabet_of(left, extra_alphabet);
        const Alphabet right_alphabet = conversion::alphabet_of(right);
        alphabet.insert(right_alphabet.begin(), right_alphabet.end());

        const spot::bdd_dict_ptr dictionary = spot::make_bdd_dict();

        const spot::twa_graph_ptr left_automaton =
            postprocess_for_comparison(conversion::to_spot_twa_raw(left, alphabet, dictionary));

        const spot::twa_graph_ptr right_automaton =
            postprocess_for_comparison(conversion::to_spot_twa_raw(right, alphabet, dictionary));

        const spot::twa_graph_ptr left_complement = complement(left_automaton);
        const spot::twa_graph_ptr right_complement = complement(right_automaton);

        const spot::twa_graph_ptr left_only = intersect(left_automaton, right_complement);
        const spot::twa_graph_ptr right_only = intersect(right_automaton, left_complement);
        const spot::twa_graph_ptr intersection = intersect(left_automaton, right_automaton);
        const spot::twa_graph_ptr neither = intersect(left_complement, right_complement);

        OmegaRegexComparison result;
        result.left_only_witness = witness(left_only, alphabet);
        result.right_only_witness = witness(right_only, alphabet);
        result.intersection_witness = witness(intersection, alphabet);
        result.neither_witness = witness(neither, alphabet);

        const bool left_only_empty = !result.left_only_witness;
        const bool right_only_empty = !result.right_only_witness;
        const bool intersection_empty = !result.intersection_witness;
        const bool neither_empty = !result.neither_witness;

        // The four regions partition the universe, so these flags need no
        // additional Spot emptiness searches.
        result.left_empty = left_only_empty && intersection_empty;
        result.right_empty = right_only_empty && intersection_empty;
        result.left_universal = right_only_empty && neither_empty;
        result.right_universal = left_only_empty && neither_empty;

        result.relation =
            classify(left_only_empty, right_only_empty, intersection_empty, neither_empty);

        return result;
    }
}
