// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L11
// expect: native-ok
// twin: tests/annotations/statements/invariant_for.cpp
// Mutation of the twin: the invariant names an enumerator; the for statement's variable hides it.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

enum { i = 3 };
int run(int n) {
  int s = 0;
  CERASTES_INVARIANT(i <= n)
  for (int i = 0; i < n; ++i) { s += i; }
  return s;
}
