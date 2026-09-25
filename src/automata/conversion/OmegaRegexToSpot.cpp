// Implements omega-regex to Spot transition-based Büchi automaton conversion.
#include "automata/conversion/OmegaRegexToSpot.hpp"

#include "automata/conversion/OmegaRegexAlphabet.hpp"
#include "automata/conversion/SpotAlphabetEncoding.hpp"
#include "automata/translation/RegexTranslation.hpp"
#include "regex/simplification/OmegaRegexSimplifier.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <spot/misc/optionmap.hh>
#include <spot/twa/twagraph.hh>
#include <spot/twaalgos/complement.hh>
#include <spot/twaalgos/postproc.hh>
#include <spot/twaalgos/product.hh>
#include <spot/twaalgos/sum.hh>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

namespace automata::conversion
{
    namespace
    {
        struct SpotContext
        {
            spot::bdd_dict_ptr dictionary;
            Alphabet alphabet;
        };

        SpotContext make_context(const Alphabet& alphabet, spot::bdd_dict_ptr dictionary)
        {
            if (alphabet.empty())
            {
                throw std::invalid_argument("Omega-word alphabet cannot be empty");
            }
            if (!dictionary)
            {
                throw std::invalid_argument("Spot BDD dictionary cannot be null");
            }

            SpotContext context;
            context.dictionary = std::move(dictionary);
            context.alphabet = alphabet;
            return context;
        }

        spot::twa_graph_ptr make_automaton(SpotContext& context)
        {
            spot::twa_graph_ptr result = spot::make_twa_graph(context.dictionary);
            spot_encoding::register_alphabet(result, context.alphabet);
            return result;
        }

        unsigned checked_state_count(const std::size_t count)
        {
            if (count > std::numeric_limits<unsigned>::max())
            {
                throw std::overflow_error("Spot automaton state count exceeds its numeric limit");
            }
            return static_cast<unsigned>(count);
        }

        spot::twa_graph_ptr empty_omega_automaton(SpotContext& context)
        {
            auto result = make_automaton(context);
            result->set_buchi();
            result->new_states(1);
            result->set_init_state(0);
            return result;
        }

        spot::twa_graph_ptr universal_omega_automaton(SpotContext& context)
        {
            auto result = make_automaton(context);
            result->set_buchi();
            result->new_states(1);
            result->set_init_state(0);

            bdd condition = bddfalse;
            for (const Symbol symbol : context.alphabet)
            {
                condition |= spot_encoding::symbol_condition(result, context.alphabet, symbol);
            }

            result->new_edge(0, 0, condition, {0});
            return result;
        }

        spot::twa_graph_ptr
        clone_into_context(const spot::twa_graph_ptr& source, SpotContext& context)
        {
            if (!source)
            {
                throw std::invalid_argument("Cannot clone null Spot automaton");
            }

            auto result = make_automaton(context);
            result->copy_acceptance_of(source);
            result->new_states(source->num_states());
            result->set_init_state(source->get_init_state_number());

            for (unsigned state = 0; state < source->num_states(); ++state)
            {
                for (const auto& edge : source->out(state))
                {
                    result->new_edge(edge.src, edge.dst, edge.cond, edge.acc);
                }
            }

            return result;
        }

        spot::twa_graph_ptr omega_power(const regex::Expression& finite, SpotContext& context)
        {
            const Nfa nfa = translation::epsilon_free_nfa(finite, context.alphabet);

            if (!nfa.is_valid() || nfa.transitions.empty())
            {
                return empty_omega_automaton(context);
            }

            // If the only accepted behavior is epsilon, no infinite word is produced.
            // This intentionally makes epsilon^omega equivalent to omega-empty.
            if (nfa.finals.contains(nfa.start))
            {
                bool has_non_epsilon_transition = false;
                for (const auto& outgoing : nfa.transitions)
                {
                    for (const Transition& transition : outgoing)
                    {
                        if (transition.symbol.has_value())
                        {
                            has_non_epsilon_transition = true;
                            break;
                        }
                    }
                }

                if (!has_non_epsilon_transition)
                {
                    return empty_omega_automaton(context);
                }
            }

            auto result = make_automaton(context);
            result->set_buchi();
            result->new_states(checked_state_count(nfa.transitions.size()));
            result->set_init_state(static_cast<unsigned>(nfa.start));

            for (StateId source = 0; source < nfa.transitions.size(); ++source)
            {
                for (const Transition& transition : nfa.transitions[source])
                {
                    if (!transition.symbol)
                    {
                        continue;
                    }

                    const bdd condition = spot_encoding::symbol_condition(
                        result, context.alphabet, *transition.symbol
                    );

                    result->new_edge(
                        static_cast<unsigned>(source),
                        static_cast<unsigned>(transition.target),
                        condition
                    );

                    if (nfa.finals.contains(transition.target))
                    {
                        result->new_edge(
                            static_cast<unsigned>(source),
                            static_cast<unsigned>(nfa.start),
                            condition,
                            {0}
                        );
                    }
                }
            }

            return result;
        }

        void copy_edges_with_offset(
            const spot::twa_graph_ptr& source, spot::twa_graph_ptr& target, unsigned offset
        )
        {
            for (unsigned state = 0; state < source->num_states(); ++state)
            {
                for (const auto& edge : source->out(state))
                {
                    target->new_edge(edge.src + offset, edge.dst + offset, edge.cond, edge.acc);
                }
            }
        }

        void copy_outgoing_edges_from_state(
            const spot::twa_graph_ptr& source,
            unsigned source_state,
            spot::twa_graph_ptr& target,
            unsigned target_state,
            unsigned target_offset
        )
        {
            for (const auto& edge : source->out(source_state))
            {
                target->new_edge(target_state, edge.dst + target_offset, edge.cond, edge.acc);
            }
        }

        spot::twa_graph_ptr concatenate(
            const regex::Expression& prefix, const spot::twa_graph_ptr& suffix, SpotContext& context
        )
        {
            const Nfa nfa = translation::epsilon_free_nfa(prefix, context.alphabet);

            if (!nfa.is_valid())
            {
                return empty_omega_automaton(context);
            }

            auto result = make_automaton(context);
            result->copy_acceptance_of(suffix);

            const unsigned prefix_offset = 0;
            const unsigned suffix_offset = static_cast<unsigned>(nfa.transitions.size());

            result->new_states(checked_state_count(nfa.transitions.size() + suffix->num_states()));
            result->set_init_state(static_cast<unsigned>(nfa.start));

            copy_edges_with_offset(suffix, result, suffix_offset);

            if (nfa.finals.contains(nfa.start))
            {
                copy_outgoing_edges_from_state(
                    suffix,
                    suffix->get_init_state_number(),
                    result,
                    static_cast<unsigned>(nfa.start),
                    suffix_offset
                );
            }

            for (StateId source = 0; source < nfa.transitions.size(); ++source)
            {
                for (const Transition& transition : nfa.transitions[source])
                {
                    if (!transition.symbol)
                    {
                        continue;
                    }

                    const bdd condition = spot_encoding::symbol_condition(
                        result, context.alphabet, *transition.symbol
                    );

                    result->new_edge(
                        static_cast<unsigned>(source + prefix_offset),
                        static_cast<unsigned>(transition.target + prefix_offset),
                        condition
                    );

                    if (nfa.finals.contains(transition.target))
                    {
                        copy_outgoing_edges_from_state(
                            suffix,
                            suffix->get_init_state_number(),
                            result,
                            static_cast<unsigned>(transition.target + prefix_offset),
                            suffix_offset
                        );
                    }
                }
            }

            return result;
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

        std::size_t automaton_size_score(const spot::twa_graph_ptr& automaton)
        {
            return static_cast<std::size_t>(automaton->num_states()) + transition_count(automaton);
        }

        spot::twa_graph_ptr postprocess_small_generic(const spot::twa_graph_ptr& automaton)
        {
            spot::postprocessor postprocessor;
            postprocessor.set_type(spot::postprocessor::Generic);
            postprocessor.set_pref(spot::postprocessor::Small);
            postprocessor.set_level(spot::postprocessor::Low);
            return postprocessor.run(automaton);
        }

        spot::twa_graph_ptr postprocess_optimized(const spot::twa_graph_ptr& automaton)
        {
            spot::postprocessor postprocessor;
            postprocessor.set_type(spot::postprocessor::Generic);
            postprocessor.set_pref(spot::postprocessor::Small);
            postprocessor.set_level(spot::postprocessor::Medium);
            return postprocessor.run(automaton);
        }

        spot::twa_graph_ptr maybe_postprocess_large(const spot::twa_graph_ptr& automaton)
        {
            constexpr std::size_t LargeIntermediateStates = 64;
            constexpr std::size_t LargeIntermediateTransitions = 512;

            if (automaton->num_states() >= LargeIntermediateStates ||
                transition_count(automaton) >= LargeIntermediateTransitions)
            {
                return postprocess_small_generic(automaton);
            }

            return automaton;
        }

        spot::twa_graph_ptr
            unite(std::vector<spot::twa_graph_ptr> automata, SpotContext& context)
        {
            if (automata.empty())
            {
                return empty_omega_automaton(context);
            }

            if (automata.size() == 1)
            {
                return clone_into_context(automata.front(), context);
            }

            std::ranges::sort(
                automata,
                [](const spot::twa_graph_ptr& left, const spot::twa_graph_ptr& right)
                { return automaton_size_score(left) < automaton_size_score(right); }
            );

            while (automata.size() > 1)
            {
                std::vector<spot::twa_graph_ptr> next;
                next.reserve((automata.size() + 1) / 2);

                for (std::size_t index = 0; index < automata.size(); index += 2)
                {
                    if (index + 1 == automata.size())
                    {
                        next.push_back(automata[index]);
                        continue;
                    }

                    next.push_back(
                        maybe_postprocess_large(spot::sum(automata[index], automata[index + 1]))
                    );
                }

                automata = std::move(next);
                std::ranges::sort(
                    automata,
                    [](const spot::twa_graph_ptr& left, const spot::twa_graph_ptr& right)
                    { return automaton_size_score(left) < automaton_size_score(right); }
                );
            }

            return automata.front();
        }

        spot::twa_graph_ptr postprocess_buchi(const spot::twa_graph_ptr& automaton)
        {
            spot::option_map options;
            // Medium otherwise skips reductions after state-based expansion.
            options.set("ba-simul", 1);
            spot::postprocessor postprocessor(&options);
            postprocessor.set_type(spot::postprocessor::BA);
            postprocessor.set_pref(spot::postprocessor::Small | spot::postprocessor::SBAcc);
            postprocessor.set_level(spot::postprocessor::Medium);
            return postprocessor.run(automaton);
        }

        spot::twa_graph_ptr
            intersect_all(std::vector<spot::twa_graph_ptr> automata, SpotContext& context)
        {
            if (automata.empty())
            {
                return universal_omega_automaton(context);
            }

            if (automata.size() == 1)
            {
                return clone_into_context(automata.front(), context);
            }

            std::ranges::sort(
                automata,
                [](const spot::twa_graph_ptr& left, const spot::twa_graph_ptr& right)
                { return automaton_size_score(left) < automaton_size_score(right); }
            );

            while (automata.size() > 1)
            {
                std::vector<spot::twa_graph_ptr> next;
                next.reserve((automata.size() + 1) / 2);

                for (std::size_t index = 0; index < automata.size(); index += 2)
                {
                    if (index + 1 == automata.size())
                    {
                        next.push_back(automata[index]);
                        continue;
                    }

                    next.push_back(
                        maybe_postprocess_large(spot::product(automata[index], automata[index + 1]))
                    );
                }

                automata = std::move(next);
                std::ranges::sort(
                    automata,
                    [](const spot::twa_graph_ptr& left, const spot::twa_graph_ptr& right)
                    { return automaton_size_score(left) < automaton_size_score(right); }
                );
            }

            return automata.front();
        }

        spot::twa_graph_ptr complement_optimized(const spot::twa_graph_ptr& automaton)
        {
            const spot::twa_graph_ptr prepared = maybe_postprocess_large(automaton);
            return maybe_postprocess_large(spot::complement(prepared));
        }

        spot::twa_graph_ptr build(const regex::omega::Expression& expression, SpotContext& context)
        {
            if (std::holds_alternative<regex::omega::EmptySet>(expression))
            {
                return empty_omega_automaton(context);
            }

            if (std::holds_alternative<regex::omega::UniversalSet>(expression))
            {
                return universal_omega_automaton(context);
            }

            if (const auto* power =
                    std::get_if<std::shared_ptr<const regex::omega::OmegaPower>>(&expression))
            {
                return omega_power((*power)->expression, context);
            }

            if (const auto* concatenation =
                    std::get_if<std::shared_ptr<const regex::omega::Concatenation>>(&expression))
            {
                return concatenate(
                    (*concatenation)->prefix, build((*concatenation)->suffix, context), context
                );
            }

            if (const auto* alternation =
                    std::get_if<std::shared_ptr<const regex::omega::Alternation>>(&expression))
            {
                std::vector<spot::twa_graph_ptr> children;
                children.reserve((*alternation)->alternatives.size());

                for (const regex::omega::Expression& child : (*alternation)->alternatives)
                {
                    children.push_back(build(child, context));
                }

                return unite(children, context);
            }

            if (const auto* intersection =
                       std::get_if<std::shared_ptr<const regex::omega::Intersection>>(&expression))
            {
                std::vector<spot::twa_graph_ptr> children;
                children.reserve((*intersection)->operands.size());

                for (const regex::omega::Expression& operand : (*intersection)->operands)
                {
                    children.push_back(build(operand, context));
                }

                return intersect_all(std::move(children), context);
            }

            if (const auto* complement =
                       std::get_if<std::shared_ptr<const regex::omega::Complement>>(&expression))
            {
                return complement_optimized(build((*complement)->expression, context));
            }

            throw std::invalid_argument("Unsupported omega regular-expression node");
        }
    }

    spot::twa_graph_ptr
    to_spot_twa_raw(const regex::omega::Expression& expression, const Alphabet& alphabet)
    {
        return to_spot_twa_raw(expression, alphabet, spot::make_bdd_dict());
    }

    spot::twa_graph_ptr to_spot_twa_raw(
        const regex::omega::Expression& expression,
        const Alphabet& alphabet,
        const spot::bdd_dict_ptr& dictionary
    )
    {
        SpotContext context = make_context(alphabet, dictionary);
        const regex::omega::Expression normalized =
            regex::simplification::omega::normalize(expression);
        return build(normalized, context);
    }

    spot::twa_graph_ptr to_spot_twa(const regex::omega::Expression& expression)
    {
        return to_spot_twa(expression, alphabet_of(expression));
    }

    spot::twa_graph_ptr
    to_spot_twa(const regex::omega::Expression& expression, const Alphabet& alphabet)
    {
        // Reduce before degeneralization/state splitting, then let Spot reduce
        // the resulting state-based automaton as well.
        return postprocess_buchi(to_optimized_spot_twa(expression, alphabet));
    }

    spot::twa_graph_ptr
        to_optimized_spot_twa(const regex::omega::Expression& expression, const Alphabet& alphabet)
    {
        return postprocess_optimized(to_spot_twa_raw(expression, alphabet));
    }
}
