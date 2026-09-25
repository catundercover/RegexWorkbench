// Implements formal automaton text formatting from graph topology.
#include "ui/graph/AutomatonText.hpp"

#include "graph/model/Layout.hpp"

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace ui::graph
{
    namespace
    {
        // UTF-8 label used for epsilon transitions in graph layouts.
        constexpr std::string_view Epsilon = "ε";

        // Appends a finite collection using mathematical set notation.
        template <typename Values>
        void append_set(std::ostringstream& output, const Values& values)
        {
            output << '{';
            bool first = true;
            for (const auto& value : values)
            {
                if (!first)
                {
                    output << ", ";
                }
                output << value;
                first = false;
            }
            output << '}';
        }

        // Splits the comma-separated labels Graphviz stores on a grouped edge.
        [[nodiscard]] std::vector<std::string> split_edge_labels(const std::string_view label)
        {
            std::vector<std::string> labels;
            std::size_t begin = 0;
            while (begin <= label.size())
            {
                const std::size_t end = label.find(',', begin);
                const std::string_view part = label.substr(
                    begin, end == std::string_view::npos ? label.size() - begin : end - begin
                );
                if (!part.empty())
                {
                    labels.emplace_back(part);
                }
                if (end == std::string_view::npos)
                {
                    break;
                }
                begin = end + 1;
            }
            return labels;
        }
    }

    std::string format_automaton_text(const ::graph::Layout& layout)
    {
        std::unordered_map<std::string, const ::graph::Node*> nodes_by_id;
        std::vector<std::string> states;
        std::vector<std::string> final_states;
        for (const ::graph::Node& node : layout.nodes)
        {
            nodes_by_id.emplace(node.id, &node);
            if (node.role != ::graph::NodeRole::State)
            {
                continue;
            }
            states.push_back(node.label);
            if (node.is_final)
            {
                final_states.push_back(node.label);
            }
        }

        std::set<std::string> alphabet;
        std::string initial_state = "∅";
        using TransitionKey = std::tuple<std::string, std::string>;
        std::map<TransitionKey, std::set<std::string>> transitions;

        for (const ::graph::Edge& edge : layout.edges)
        {
            const auto source = nodes_by_id.find(edge.source);
            const auto target = nodes_by_id.find(edge.target);
            if (source == nodes_by_id.end() || target == nodes_by_id.end())
            {
                continue;
            }
            if (source->second->role == ::graph::NodeRole::StartMarker)
            {
                initial_state = target->second->label;
                continue;
            }
            if (source->second->role != ::graph::NodeRole::State ||
                target->second->role != ::graph::NodeRole::State)
            {
                continue;
            }

            for (const std::string& symbol : split_edge_labels(edge.label))
            {
                transitions[{source->second->label, symbol}].insert(target->second->label);
                if (symbol != Epsilon)
                {
                    alphabet.insert(symbol);
                }
            }
        }

        std::ostringstream output;
        output << "A = (Q, Σ, δ, q₀, F)\n";
        output << "Q = ";
        append_set(output, states);
        output << "\nΣ = ";
        append_set(output, alphabet);
        output << "\nq₀ = " << initial_state << "\nF = ";
        append_set(output, final_states);
        output << "\n\n";

        if (transitions.empty())
        {
            output << "δ = ∅\n";
            return output.str();
        }

        output << "δ:\n";
        for (const auto& [key, targets] : transitions)
        {
            const auto& [source, symbol] = key;
            output << "  δ(" << source << ", " << symbol << ") = ";
            append_set(output, targets);
            output << '\n';
        }
        return output.str();
    }
}
