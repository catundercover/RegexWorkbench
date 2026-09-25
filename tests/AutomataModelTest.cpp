// Verifies NFA construction, validation, and acceptance behavior.
#include "automata/model/Nfa.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
    // Reports a failed assertion and returns its condition.
    bool check(bool condition, const std::string& message)
    {
        if (!condition)
        {
            std::cerr << message << '\n';
        }
        return condition;
    }
}

// Runs NFA construction, validity, and acceptance checks.
int main()
{
    automata::Nfa nfa;
    const automata::StateId start = nfa.add_state();
    const automata::StateId after_epsilon = nfa.add_state();
    const automata::StateId final = nfa.add_state();

    nfa.start = start;
    nfa.finals.insert(final);
    nfa.add_epsilon_transition(start, after_epsilon);
    nfa.add_transition(after_epsilon, 'a', final);
    nfa.add_transition(final, 'b', final);

    if (!check(nfa.is_valid(), "A well-formed NFA was rejected.") ||
        !check(nfa.accepts("a"), "The NFA rejected its shortest word.") ||
        !check(nfa.accepts("abbb"), "The NFA rejected a word using its loop.") ||
        !check(!nfa.accepts(""), "The NFA incorrectly accepted epsilon.") ||
        !check(!nfa.accepts("b"), "The NFA skipped a required transition.") ||
        !check(nfa.alphabet == automata::Alphabet{'a', 'b'}, "The alphabet was not maintained."))
    {
        return 1;
    }

    automata::Nfa invalid;
    if (!check(!invalid.is_valid(), "An NFA without states was considered valid."))
    {
        return 1;
    }

    bool invalid_evaluation_rejected = false;
    try
    {
        (void)invalid.accepts("");
    }
    catch (const std::invalid_argument&)
    {
        invalid_evaluation_rejected = true;
    }
    if (!check(invalid_evaluation_rejected, "Evaluating an invalid NFA did not fail."))
    {
        return 1;
    }

    return 0;
}
