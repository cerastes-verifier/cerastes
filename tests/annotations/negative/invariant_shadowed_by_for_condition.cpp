// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L12
// expect: native-ok
// twin: tests/annotations/statements/invariant_for.cpp
// Mutation of the twin: the same for a declaration in the for statement's condition.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int count(int n) CERASTES_REQUIRES(n >= 0)
{
  [[maybe_unused]] int i;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && c == i) for (; int i = n - c; ) {
    c += i > 0 ? 1 : 0;
  }
  return c;
}
