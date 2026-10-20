// expect: golden
// expect: native-ok
// Pins: forall over an array of T in the contract of a function template (the lambda names the
// function's own T): dependent in the prototype (a const T * lambda parameter), substituted in
// each instance (int and long). Moved from the negatives (owner, 2026-10-16): since the fork's
// series L/M the lambda may name T in a function template's contract (decision 0006 item 9).
// Twins: tests/annotations/negative/forall_function_template_decltype.cpp (the lambda depends on
// T only through decltype of a parameter) and the class-template twin forall_class_template.cpp.
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
