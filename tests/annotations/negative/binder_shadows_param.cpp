// expect: frontend-error decl_hides_function_parameter @ L13
// expect: native-ok
// twin: tests/annotations/contracts/ensures_binder.cpp
// Mutation of the twin: the result name x is also a parameter name.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int abs_val(int x) CERASTES_ENSURES(r, r >= 0 && (r == x || r == -x))
{
  return x < 0 ? -x : x;
}

long widen(int x) CERASTES_ENSURES(x, x == x);
