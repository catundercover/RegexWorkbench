// Implements a total, disjoint encoding of character alphabets as Spot valuations.
#include "automata/conversion/SpotAlphabetEncoding.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace automata::conversion::spot_encoding
{
    namespace
    {
        // Prefix reserved for internal letter-encoding atomic propositions.
        constexpr const char* BitNamePrefix = "__regexthesis_letter_bit_";

        std::vector<Symbol> ordered_symbols(const Alphabet& alphabet)
        {
            std::vector<Symbol> symbols(alphabet.begin(), alphabet.end());
            std::ranges::sort(symbols);
            return symbols;
        }

        std::size_t required_bits(const std::size_t symbol_count)
        {
            std::size_t bits = 0;
            std::size_t capacity = 1;
            while (capacity < symbol_count)
            {
                capacity *= 2;
                ++bits;
            }
            return bits;
        }

        std::vector<int>
        register_bits(const spot::twa_graph_ptr& automaton, const std::size_t symbol_count)
        {
            if (!automaton)
            {
                throw std::invalid_argument("Cannot encode an alphabet in a null Spot automaton");
            }
            if (symbol_count == 0)
            {
                throw std::invalid_argument("Cannot encode an empty omega-word alphabet");
            }

            const std::size_t bit_count = required_bits(symbol_count);
            std::vector<int> variables;
            variables.reserve(bit_count);
            for (std::size_t bit = 0; bit < bit_count; ++bit)
            {
                variables.push_back(
                    automaton->register_ap(std::string(BitNamePrefix) + std::to_string(bit))
                );
            }
            return variables;
        }

        bdd exact_code(const std::vector<int>& variables, const std::size_t code)
        {
            bdd result = bddtrue;
            for (std::size_t bit = 0; bit < variables.size(); ++bit)
            {
                result &= (code & (std::size_t{1} << bit)) != 0 ? bdd_ithvar(variables[bit])
                                                                : bdd_nithvar(variables[bit]);
            }
            return result;
        }

        bdd condition_at(
            const std::vector<int>& variables,
            const std::size_t position,
            const std::size_t symbol_count
        )
        {
            if (position + 1 < symbol_count)
            {
                return exact_code(variables, position);
            }

            // Give every otherwise-unused valuation to the last symbol. This
            // partitions bddtrue, so Spot complement remains relative to the
            // project's character alphabet without a separate domain automaton.
            bdd earlier_symbols = bddfalse;
            for (std::size_t earlier = 0; earlier < position; ++earlier)
            {
                earlier_symbols |= exact_code(variables, earlier);
            }
            return !earlier_symbols;
        }
    }

    void register_alphabet(const spot::twa_graph_ptr& automaton, const Alphabet& alphabet)
    {
        (void)register_bits(automaton, alphabet.size());
    }

    bdd symbol_condition(
        const spot::twa_graph_ptr& automaton, const Alphabet& alphabet, const Symbol symbol
    )
    {
        const std::vector<Symbol> symbols = ordered_symbols(alphabet);
        const auto found = std::ranges::find(symbols, symbol);
        if (found == symbols.end())
        {
            throw std::invalid_argument("Spot symbol is not part of the active alphabet");
        }

        const std::vector<int> variables = register_bits(automaton, symbols.size());
        return condition_at(
            variables,
            static_cast<std::size_t>(std::distance(symbols.begin(), found)),
            symbols.size()
        );
    }

    std::vector<Symbol> satisfying_symbols(
        const spot::twa_graph_ptr& automaton, const Alphabet& alphabet, const bdd& condition
    )
    {
        const std::vector<Symbol> symbols = ordered_symbols(alphabet);
        const std::vector<int> variables = register_bits(automaton, symbols.size());

        std::vector<Symbol> result;
        for (std::size_t position = 0; position < symbols.size(); ++position)
        {
            if ((condition & condition_at(variables, position, symbols.size())) != bddfalse)
            {
                result.push_back(symbols[position]);
            }
        }
        return result;
    }
}
