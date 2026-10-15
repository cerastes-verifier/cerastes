// expect: frontend-error cerastes-annotation_side_effect @ L8
// expect: native-ok
// twin: tests/annotations/contracts/ensures_binder.cpp
// Mutation of the twin: assignment to a parameter in a postcondition.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int abs_val(int x) CERASTES_ENSURES(r, r >= 0 && (r == (x = 1) || r == -x))
{
  return x < 0 ? -x : x;
}

long widen(int x) CERASTES_ENSURES(r, r == x);
