// Declares conversions between project automata and MATA representations.
#pragma once

#include "automata/model/Nfa.hpp"

#include <mata/nfa/nfa.hh>
#include <mata/utils/ord-vector.hh>
#include <optional>
#include <string>

namespace automata::conversion::detail
{
    // Converts a valid project NFA to MATA's representation.
    [[nodiscard]] mata::nfa::Nfa to_mata(const Nfa& nfa);
    // Converts a MATA automaton to the project representation.
    [[nodiscard]] Nfa from_mata(const mata::nfa::Nfa& source);
    // Removes epsilon transitions, determinizes, and minimizes a MATA NFA.
    [[nodiscard]] mata::nfa::Nfa minimize(const mata::nfa::Nfa& nfa);
    // Converts a project alphabet to MATA's ordered symbol collection.
    [[nodiscard]] mata::utils::OrdVector<mata::Symbol> to_mata_alphabet(const Alphabet& alphabet);
    // Returns a shortest accepted word, or no value for an empty language.
    [[nodiscard]] std::optional<std::string> shortest_word(const mata::nfa::Nfa& nfa);
}
