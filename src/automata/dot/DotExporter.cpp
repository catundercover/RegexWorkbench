// Implements stable Graphviz DOT serialization for automata.
#include "automata/dot/DotExporter.hpp"

#include <cctype>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace automata::dot
{
    namespace
    {
        // UTF-8 label used for epsilon transitions.
        constexpr const char* Epsilon = "\xCE\xB5";

        // Escapes text for a quoted Graphviz attribute value.
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

        // Formats a symbol or epsilon as a stable printable edge label.
        std::string label_of(const std::optional<Symbol>& symbol)
        {
            if (!symbol)
                return Epsilon;

            const unsigned char character = static_cast<unsigned char>(*symbol);
            if (std::isprint(character))
                return std::string(1, static_cast<char>(character));

            std::ostringstream label;
            label << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                  << static_cast<int>(character);
            return label.str();
        }
    }

    std::string to_dot(const Nfa& nfa)
    {
        if (!nfa.is_valid())
        {
            throw std::invalid_argument("Cannot export an invalid NFA");
        }

        std::ostringstream output;
        output << "digraph Automaton {\n"
               << "  rankdir=LR;\n"
               << "  node [shape=circle, style=filled, fillcolor=\"#4296FA\", "
                  "fontname=\"Arial\", fontsize=\"12\"];\n"
               << "  edge [fontname=\"Arial\", fontsize=\"10\"];\n"
               << "  __start [shape=point, label=\"\", width=0.1, height=0.1];\n\n";

        for (StateId state = 0; state < nfa.transitions.size(); ++state)
        {
            output << "  node" << state << " [label=\"" << state << "\"";
            if (nfa.finals.contains(state))
                output << ", shape=doublecircle";
            output << "];\n";
        }
        output << "  __start -> node" << nfa.start << ";\n\n";

        // Parallel transitions share one edge so layout complexity matches what the UI renders.
        std::map<std::pair<StateId, StateId>, std::set<std::string>> grouped_labels;
        for (StateId source = 0; source < nfa.transitions.size(); ++source)
        {
            for (const Transition& transition : nfa.transitions[source])
            {
                grouped_labels[{source, transition.target}].insert(label_of(transition.symbol));
            }
        }

        for (const auto& [states, labels] : grouped_labels)
        {
            std::string combined;
            for (const std::string& label : labels)
            {
                if (!combined.empty())
                    combined += ',';
                combined += label;
            }

            output << "  node" << states.first << " -> node" << states.second << " [label=\""
                   << escape(combined) << "\"";
            if (combined == Epsilon)
                output << ", style=dashed";
            output << "];\n";
        }

        output << "}\n";
        return output.str();
    }
}
