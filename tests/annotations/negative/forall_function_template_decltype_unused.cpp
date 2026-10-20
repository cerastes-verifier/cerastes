// expect: frontend-error cerastes-annotation_substitution_error @ L9
// expect: native-ok
// twin: tests/annotations/spec_functions/forall_function_template.cpp
// Mutation of the twin: the lambda's array parameter is declared decltype(data) and the body does not use it (the substituted parameter has an error type and EDG reports nothing; only the lambda's own signature shows it).
// Pins: the lambda routine's parameter types are checked, not only the nodes of its body; one error, at the contract's line.
#include <cerastes.h>

template <class T>
bool all_pos([[maybe_unused]] T* data, [[maybe_unused]] int n) CERASTES_REQUIRES(cerastes::forall([](int i, decltype(data) a, int m) { return !(0 <= i && i < m); }, data, n))
{
  return true;
}

bool use(int* p)
{
  return all_pos(p, 1);
}
