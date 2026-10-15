// expect: frontend-error undefined_identifier @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_def.cpp
// Mutation of the twin: every attribute in a chain is checked, not just the first.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int clamp_pos(int x, int hi) CERASTES_REQUIRES(hi > 0) CERASTES_REQUIRES(lo >= 0)
{
  return x < hi ? x : hi;
}
