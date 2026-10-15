// expect: frontend-error attr_redeclaration_mismatch @ L10
// expect: native-ok
// twin: tests/annotations/contracts/definition_inherits.cpp
// Mutation of the twin: the definition repeats only the requires of a header declaration that has requires and ensures.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int halve(int x) CERASTES_REQUIRES(x >= 0) CERASTES_ENSURES(r, r <= x);

int halve(int x) CERASTES_REQUIRES(x >= 0)
{
  return x / 2;
}
