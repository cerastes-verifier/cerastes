// expect: frontend-error exp_rparen @ L14
// expect: frontend-error exp_comma @ L14
// expect: native-ok
// twin: tests/annotations/contracts/ensures_binder.cpp
// Mutation of the twin: ensures with a result name and two conditions: the second comma is a parse error.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int abs_val(int x) CERASTES_ENSURES(r, r >= 0 && (r == x || r == -x))
{
  return x < 0 ? -x : x;
}

long widen(int x) CERASTES_ENSURES(r, r == x, r > 0);
