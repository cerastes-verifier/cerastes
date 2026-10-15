// expect: frontend-error exp_rparen @ L9
// expect: frontend-error exp_comma @ L9
// expect: native-ok
// twin: tests/annotations/contracts/ensures_plain.cpp
// Mutation of the twin: the first argument is not a lone identifier, so it is no result name and the comma ends the (comma-free) condition: two parse errors, as for requires with two arguments.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int next(int x) CERASTES_ENSURES(x < 1000, x > 0)
{
  x = x + 1;
  return x;
}
