// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L12
// expect: native-ok
// twin: tests/annotations/statements/invariant_while.cpp
// Mutation of the twin: a variable declared in the loop's condition shadows the one the invariant names.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int sum_to(int n) CERASTES_REQUIRES(n >= 0)
{
  [[maybe_unused]] int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s >= 0) while (int i = n - s) {
    s += i;
  }
  return s;
}
