// expect: frontend-error undefined_identifier @ L9
// expect: native-ok
// twin: tests/annotations/spec_functions/forall_class_template.cpp
// Mutation of the twin: the same forall, with the lambda naming T, in a function template's contract: EDG reports T as undefined (decision 0006 records the limitation; the class-template form is the supported one).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
bool all_pos([[maybe_unused]] T* data, [[maybe_unused]] int n) CERASTES_REQUIRES(cerastes::forall([](int i, const T* a, int m) { return !(0 <= i && i < m) || a[i] > 0; }, data, n))
{
  return true;
}

bool use(int* p, long* q)
{
  return all_pos(p, 1) && all_pos(q, 1);
}
