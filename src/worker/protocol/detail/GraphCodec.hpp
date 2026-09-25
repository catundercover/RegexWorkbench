// Declares binary encoding for graph layouts and comparison results.
#pragma once

#include "automata/analysis/OmegaRegexComparison.hpp"
#include "automata/analysis/RegexComparison.hpp"
#include "graph/model/Layout.hpp"

namespace worker::protocol::detail
{
    class Reader;
    class Writer;

    // Serializes a complete graph layout.
    void write_layout(Writer& writer, const graph::Layout& layout);
    // Decodes and validates a complete graph layout.
    [[nodiscard]] graph::Layout read_layout(Reader& reader);

    // Serializes a semantic regular-language comparison.
    void write_comparison(Writer& writer, const automata::analysis::RegexComparison& comparison);
    // Decodes and validates a semantic regular-language comparison.
    [[nodiscard]] automata::analysis::RegexComparison read_comparison(Reader& reader);

    // Serializes a semantic omega-regular-language comparison.
    void write_omega_comparison(
        Writer& writer, const automata::analysis::OmegaRegexComparison& comparison
    );
    // Decodes and validates a semantic omega-regular-language comparison.
    [[nodiscard]] automata::analysis::OmegaRegexComparison read_omega_comparison(Reader& reader);
}
