// expect: frontend-error attr_redeclaration_mismatch @ L10
// expect: native-ok
// twin: tests/annotations/contracts/redecl_same.cpp
// Mutation of the twin: a redeclaration that carries a contract must carry the whole one (only a declaration with none is fine).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int bump(int x) CERASTES_REQUIRES(x > 0 && x < 100) CERASTES_ENSURES(r, r > x);
int bump(int y);
int bump(int z) CERASTES_REQUIRES(z > 0 && z < 100)
{
  return z + 1;
}
