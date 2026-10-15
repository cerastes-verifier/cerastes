// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L12
// expect: native-ok
// twin: tests/annotations/statements/invariant_range_for.cpp
// Mutation of the twin: the same for the variable of a range-based for statement.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

struct total_of {
  int x = 0;
  int run(const int (&a)[3], int n) {
    int s = 0;
    CERASTES_INVARIANT(x <= n)
    for (int x : a) { s += x; }
    return s + n;
  }
};
