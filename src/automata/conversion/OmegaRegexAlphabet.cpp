// Implements terminal-alphabet extraction from omega regular-expression trees.
#include "automata/conversion/OmegaRegexAlphabet.hpp"

#include "automata/conversion/RegexAlphabet.hpp"

#include <memory>
#include <type_traits>
#include <variant>

namespace automata::conversion
{
    namespace
    {
        void merge(Alphabet& target, const Alphabet& source)
        {
            target.insert(source.begin(), source.end());
        }

        void collect(const regex::omega::Expression& expression, Alphabet& result)
        {
            if (std::holds_alternative<regex::omega::EmptySet>(expression) ||
                    std::holds_alternative<regex::omega::UniversalSet>(expression))
            {
                return;
            }

            if (const auto* power =
                    std::get_if<std::shared_ptr<const regex::omega::OmegaPower>>(&expression))
            {
                merge(result, alphabet_of((*power)->expression));
                return;
            }

            if (const auto* concatenation =
                    std::get_if<std::shared_ptr<const regex::omega::Concatenation>>(&expression))
            {
                merge(result, alphabet_of((*concatenation)->prefix));
                collect((*concatenation)->suffix, result);
                return;
            }

            if (const auto* complement =
                    std::get_if<std::shared_ptr<const regex::omega::Complement>>(&expression))
            {
                collect((*complement)->expression, result);
                return;
            }

            if (const auto* alternation =
                    std::get_if<std::shared_ptr<const regex::omega::Alternation>>(&expression))
            {
                for (const regex::omega::Expression& child : (*alternation)->alternatives)
                {
                    collect(child, result);
                }
                return;
            }

            if (const auto* intersection =
                    std::get_if<std::shared_ptr<const regex::omega::Intersection>>(&expression))
            {
                for (const regex::omega::Expression& child : (*intersection)->operands)
                {
                    collect(child, result);
                }
            }
        }
    }

    Alphabet alphabet_of(const regex::omega::Expression& expression)
    {
        Alphabet result;
        collect(expression, result);
        return result;
    }

    Alphabet alphabet_of(const regex::omega::Expression& expression, const Alphabet& extra_alphabet)
    {
        Alphabet result = alphabet_of(expression);
        result.insert(extra_alphabet.begin(), extra_alphabet.end());
        return result;
    }
}
