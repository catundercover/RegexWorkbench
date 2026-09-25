// Declares structural inspection helpers for omega expression trees.
#pragma once

#include "regex/model/OmegaExpression.hpp"

#include <cstddef>
#include <set>

namespace regex::inspection::omega
{
    // Compares two omega expression trees without applying language identities.
    [[nodiscard]] bool
    structurally_equal(const regex::omega::Expression& left, const regex::omega::Expression& right);
    // Collects distinct finite terminal symbols occurring anywhere in the omega expression.
    [[nodiscard]] std::set<char> terminals(const regex::omega::Expression& expression);

    // Returns whether the omega expression references the active alphabet via Σ or Σ^ω.
    [[nodiscard]] bool contains_any_symbol(const regex::omega::Expression& expression);

    // Counts the root and every finite and omega AST node below it.
    [[nodiscard]] std::size_t node_count(const regex::omega::Expression& expression);
}
