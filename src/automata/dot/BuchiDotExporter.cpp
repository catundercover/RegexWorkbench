// Implements stable Graphviz DOT serialization for state-based Büchi automata.
#include "automata/dot/BuchiDotExporter.hpp"

#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace automata::dot
{
    namespace
    {
        std::string escape(std::string_view input)
        {
            std::string result;
            result.reserve(input.size());

            for (char character : input)
            {
                switch (character)
                {
                case '\\':
                    result += "\\\\";
                    break;
                case '"':
                    result += "\\\"";
                    break;
                case '\n':
                    result += "\\n";
                    break;
                case '\t':
                    result += "\\t";
                    break;
                default:
                    result += character;
                    break;
                }
            }

            return result;
        }

        std::string transition_label(const Alphabet& symbols, const Alphabet& complete_alphabet)
        {
            if (symbols == complete_alphabet)
            {
                return "Σ";
            }

            std::vector<Symbol> ordered(symbols.begin(), symbols.end());
            std::ranges::sort(ordered);

            std::string result;
            for (const Symbol symbol : ordered)
            {
                if (!result.empty())
                {
                    result += ',';
                }
                result += symbol;
            }
            return result;
        }
    }

    std::string to_dot(const BuchiAutomaton& automaton)
    {
        if (!automaton.is_valid())
        {
            throw std::invalid_argument("Cannot export an invalid Büchi automaton");
        }

        std::ostringstream output;
        output << "digraph BuchiAutomaton {\n"
               << "  rankdir=LR;\n"
               << "  node [shape=circle, style=filled, fillcolor=\"#4296FA\", "
                  "fontname=\"Arial\", fontsize=\"12\"];\n"
               << "  edge [fontname=\"Arial\", fontsize=\"10\"];\n"
               << "  __start [shape=point, label=\"\", width=0.1, height=0.1];\n\n";

        for (StateId state = 0; state < automaton.transitions.size(); ++state)
        {
            output << "  node" << state << " [label=\"" << state << "\"";

            if (automaton.finals.contains(state))
            {
                output << ", shape=doublecircle";
            }

            output << "];\n";
        }

        output << "  __start -> node" << automaton.start << ";\n\n";

        std::map<std::pair<StateId, StateId>, Alphabet> grouped_symbols;

        for (StateId source = 0; source < automaton.transitions.size(); ++source)
        {
            for (const BuchiTransition& transition : automaton.transitions[source])
            {
                auto& symbols = grouped_symbols[{source, transition.target}];
                symbols.insert(transition.symbols.begin(), transition.symbols.end());
            }
        }

        for (const auto& [states, symbols] : grouped_symbols)
        {
            output << "  node" << states.first << " -> node" << states.second << " [label=\""
                   << escape(transition_label(symbols, automaton.alphabet)) << "\"];\n";
        }

        output << "}\n";
        return output.str();
    }
}
