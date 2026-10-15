// expect: frontend-error expr_not_class @ L17
// expect: native-ok
// twin: tests/annotations/statements/template_statements.cpp
// Mutation of the twin: dependent in the prototype, an error only in the instance for T=int, with the instantiation context.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
T sum_n(T n) CERASTES_REQUIRES(n >= 0)
{
  T i = 0;
  T s = 0;
  CERASTES_INVARIANT(i >= 0 && i <= n && s >= 0) while (i < n) {
    s += i;
    i += 1;
  }
  CERASTES_CHECK(s.x >= 0);
  return s;
}

int use(int n)
{
  return sum_n(n);
}
