// expect: frontend-error attr_redeclaration_mismatch @ L17
// expect: native-ok
// twin: tests/annotations/contracts/ensures_named_redecl.cpp
// Mutation of the twin: the declaration's ensures names the result, the definition's does not.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int f(int x) CERASTES_ENSURES(r, r > x);

int f(int y) CERASTES_ENSURES(res, res > y)
{
  return y + 1;
}

int g(int x) CERASTES_ENSURES(r, x < 100);

int g(int y) CERASTES_ENSURES(y < 100)
{
  return y;
}
