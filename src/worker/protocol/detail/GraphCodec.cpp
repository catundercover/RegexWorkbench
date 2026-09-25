// Implements binary serialization of graph and comparison models.
#include "worker/protocol/detail/GraphCodec.hpp"

#include "worker/protocol/detail/BinaryCodec.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace worker::protocol::detail
{
    namespace
    {
        // Serializes one two-dimensional graph point.
        void write_point(Writer& writer, const graph::Point point)
        {
            writer.write_float(point.x);
            writer.write_float(point.y);
        }

        // Decodes one finite two-dimensional graph point.
        [[nodiscard]] graph::Point read_point(Reader& reader)
        {
            return graph::Point{reader.read_float(), reader.read_float()};
        }
    }

    void write_layout(Writer& writer, const graph::Layout& layout)
    {
        write_point(writer, layout.bounds.minimum);
        write_point(writer, layout.bounds.maximum);

        writer.write_count(layout.nodes.size());
        for (const graph::Node& node : layout.nodes)
        {
            writer.write_string(node.id);
            writer.write_string(node.label);
            write_point(writer, node.position);
            writer.write_float(node.radius);
            writer.write_bool(node.is_final);
            writer.write_u8(static_cast<std::uint8_t>(node.role));
        }

        writer.write_count(layout.edges.size());
        for (const graph::Edge& edge : layout.edges)
        {
            writer.write_string(edge.source);
            writer.write_string(edge.target);
            writer.write_string(edge.label);
            writer.write_count(edge.spline.size());
            for (const graph::CubicBezier& segment : edge.spline)
            {
                write_point(writer, segment.start);
                write_point(writer, segment.first_control);
                write_point(writer, segment.second_control);
                write_point(writer, segment.end);
            }
            writer.write_optional(
                edge.arrow_tip, [&](const graph::Point point) { write_point(writer, point); }
            );
            writer.write_optional(
                edge.label_position, [&](const graph::Point point) { write_point(writer, point); }
            );
            writer.write_u8(static_cast<std::uint8_t>(edge.label_alignment));
        }
    }

    graph::Layout read_layout(Reader& reader)
    {
        graph::Layout layout;
        layout.bounds = graph::Bounds{read_point(reader), read_point(reader)};

        const std::size_t node_count = reader.read_count();
        layout.nodes.reserve(node_count);
        for (std::size_t index = 0; index < node_count; ++index)
        {
            graph::Node node;
            node.id = reader.read_string();
            node.label = reader.read_string();
            node.position = read_point(reader);
            node.radius = reader.read_float();
            node.is_final = reader.read_bool();
            const std::uint8_t role = reader.read_u8();
            if (role > static_cast<std::uint8_t>(graph::NodeRole::StartMarker))
            {
                throw std::runtime_error("Protocol node role is invalid.");
            }
            node.role = static_cast<graph::NodeRole>(role);
            layout.nodes.push_back(std::move(node));
        }

        const std::size_t edge_count = reader.read_count();
        layout.edges.reserve(edge_count);
        for (std::size_t index = 0; index < edge_count; ++index)
        {
            graph::Edge edge;
            edge.source = reader.read_string();
            edge.target = reader.read_string();
            edge.label = reader.read_string();
            const std::size_t segment_count = reader.read_count();
            edge.spline.reserve(segment_count);
            for (std::size_t segment = 0; segment < segment_count; ++segment)
            {
                edge.spline.push_back(
                    graph::CubicBezier{
                        read_point(reader),
                        read_point(reader),
                        read_point(reader),
                        read_point(reader)
                    }
                );
            }
            edge.arrow_tip = reader.read_optional([&reader] { return read_point(reader); });
            edge.label_position = reader.read_optional([&reader] { return read_point(reader); });
            const std::uint8_t alignment = reader.read_u8();
            if (alignment > static_cast<std::uint8_t>(graph::LabelAlignment::Right))
            {
                throw std::runtime_error("Protocol label alignment is invalid.");
            }
            edge.label_alignment = static_cast<graph::LabelAlignment>(alignment);
            layout.edges.push_back(std::move(edge));
        }
        return layout;
    }

    void write_comparison(Writer& writer, const automata::analysis::RegexComparison& comparison)
    {
        writer.write_u8(static_cast<std::uint8_t>(comparison.relation));
        const auto write_optional_string = [&writer](const std::optional<std::string>& value)
        {
            writer.write_optional(
                value, [&writer](const std::string& text) { writer.write_string(text); }
            );
        };
        write_optional_string(comparison.left_only_witness);
        write_optional_string(comparison.right_only_witness);
        write_optional_string(comparison.intersection_witness);
        write_optional_string(comparison.neither_witness);
        writer.write_bool(comparison.left_empty);
        writer.write_bool(comparison.right_empty);
        writer.write_bool(comparison.left_universal);
        writer.write_bool(comparison.right_universal);
    }

    automata::analysis::RegexComparison read_comparison(Reader& reader)
    {
        automata::analysis::RegexComparison comparison;
        const std::uint8_t relation = reader.read_u8();
        if (relation > static_cast<std::uint8_t>(automata::analysis::LanguageRelation::Overlap))
        {
            throw std::runtime_error("Protocol language relation is invalid.");
        }
        comparison.relation = static_cast<automata::analysis::LanguageRelation>(relation);
        const auto read_optional_string = [&reader]
        { return reader.read_optional([&reader] { return reader.read_string(); }); };
        comparison.left_only_witness = read_optional_string();
        comparison.right_only_witness = read_optional_string();
        comparison.intersection_witness = read_optional_string();
        comparison.neither_witness = read_optional_string();
        comparison.left_empty = reader.read_bool();
        comparison.right_empty = reader.read_bool();
        comparison.left_universal = reader.read_bool();
        comparison.right_universal = reader.read_bool();
        return comparison;
    }

    void write_omega_comparison(
        Writer& writer, const automata::analysis::OmegaRegexComparison& comparison
    )
    {
        writer.write_u8(static_cast<std::uint8_t>(comparison.relation));

        const auto write_optional_witness =
            [&writer](const std::optional<automata::analysis::OmegaWitness>& value)
        {
            writer.write_optional(
                value,
                [&writer](const automata::analysis::OmegaWitness& witness)
                {
                    writer.write_string(witness.prefix);
                    writer.write_string(witness.cycle);
                }
            );
        };

        write_optional_witness(comparison.left_only_witness);
        write_optional_witness(comparison.right_only_witness);
        write_optional_witness(comparison.intersection_witness);
        write_optional_witness(comparison.neither_witness);
        writer.write_bool(comparison.left_empty);
        writer.write_bool(comparison.right_empty);
        writer.write_bool(comparison.left_universal);
        writer.write_bool(comparison.right_universal);
    }

    automata::analysis::OmegaRegexComparison read_omega_comparison(Reader& reader)
    {
        automata::analysis::OmegaRegexComparison comparison;
        const std::uint8_t relation = reader.read_u8();
        if (relation > static_cast<std::uint8_t>(automata::analysis::LanguageRelation::Overlap))
        {
            throw std::runtime_error("Protocol omega language relation is invalid.");
        }

        comparison.relation = static_cast<automata::analysis::LanguageRelation>(relation);

        const auto read_optional_witness = [&reader]
        {
            return reader.read_optional(
                [&reader]
                {
                    automata::analysis::OmegaWitness witness;
                    witness.prefix = reader.read_string();
                    witness.cycle = reader.read_string();
                    if (witness.cycle.empty())
                    {
                        throw std::runtime_error("Protocol omega witness cycle is empty.");
                    }
                    return witness;
                }
            );
        };

        comparison.left_only_witness = read_optional_witness();
        comparison.right_only_witness = read_optional_witness();
        comparison.intersection_witness = read_optional_witness();
        comparison.neither_witness = read_optional_witness();
        comparison.left_empty = reader.read_bool();
        comparison.right_empty = reader.read_bool();
        comparison.left_universal = reader.read_bool();
        comparison.right_universal = reader.read_bool();
        return comparison;
    }
}
