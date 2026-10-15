// expect: frontend-error exp_primary_expr @ L13
// expect: native-ok
// twin: tests/annotations/contracts/ensures_binder.cpp
// Mutation of the twin: ensures with no argument (read from the fork without commit E; re-check after E).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int abs_val(int x) CERASTES_ENSURES(r, r >= 0 && (r == x || r == -x))
{
  return x < 0 ? -x : x;
}

long widen(int x) CERASTES_ENSURES();
