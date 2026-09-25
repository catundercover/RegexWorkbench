// Declares configurable rewriting for omega regular expressions.
#pragma once

#include "automata/model/Nfa.hpp"
#include "automata/rewrite/RegexRewriter.hpp"
#include "regex/model/OmegaExpression.hpp"

#include <string>

namespace automata::rewrite::omega
{
    // Omega rewriting uses the same operator-removal switches as finite rewriting.
    using Options = automata::rewrite::Options;

    // Normalizes and rewrites an omega expression, returning canonical parseable syntax.
    [[nodiscard]] std::string apply(
        const regex::omega::Expression& expression,
        const Options& options,
        const Alphabet& extra_alphabet = {}
    );
}
