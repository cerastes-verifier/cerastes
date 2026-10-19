// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L13
// expect: native-ok
// twin: tests/annotations/statements/decreases_for_do.cpp
// Mutation of the twin: the loop declares its own i: the measure would bind the outer one.
// Pins: 0006 item 2 for the loop form of decreases
#include <cerastes.h>


int count(int n) CERASTES_REQUIRES(n >= 0)
{
  [[maybe_unused]] int i;
  int c = 0;
  CERASTES_DECREASES(n - i) for (int i = 0; i < n; ++i) {
    c += 1;
  }
  return c;
}
