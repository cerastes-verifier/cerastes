// expect: frontend-error cerastes-annotation_substitution_error @ L10
// expect: frontend-error cerastes-unsupported_annotation_expression @ L10
// expect: native-ok
// twin: tests/annotations/spec_functions/forall_function_template.cpp
// Mutation of the twin: the lambda's array parameter is declared decltype(data) instead of const T * (it depends on T only through decltype of a function parameter; handoff 5 item 2).
// Pins: this limit stays rejected after the fork's series L/M: the substituted annotation is erroneous (substitution error, then the walker's refusal of the error node; both at the contract's line).
#include <cerastes.h>

template <class T>
bool all_pos([[maybe_unused]] T* data, [[maybe_unused]] int n) CERASTES_REQUIRES(cerastes::forall([](int i, decltype(data) a, int m) { return !(0 <= i && i < m) || a[i] > 0; }, data, n))
{
  return true;
}

bool use(int* p)
{
  return all_pos(p, 1);
}
