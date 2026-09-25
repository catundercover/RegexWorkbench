// Declares structural queries over regular-expression trees.
#pragma once

#include "regex/model/Expression.hpp"

#include <cstddef>
#include <set>

namespace regex::inspection
{
    // Returns whether two trees contain the same node kinds and values.
    [[nodiscard]] bool structurally_equal(const Expression& left, const Expression& right);
    // Returns whether any terminal node occurs in the tree.
    [[nodiscard]] bool contains_terminal(const Expression& expression);
    // Returns whether the expression explicitly references the active alphabet via Σ.
    [[nodiscard]] bool contains_any_symbol(const Expression& expression);
    // Collects the distinct terminal symbols occurring in the tree.
    [[nodiscard]] std::set<char> terminals(const Expression& expression);
    // Counts occurrences in the expression tree, including repeated shared subexpressions.
    [[nodiscard]] std::size_t node_count(const Expression& expression);
}
