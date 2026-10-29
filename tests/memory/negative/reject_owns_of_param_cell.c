// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owns_inc.c
// Pins: 0037 items 5 and 15: owns of a place other than *param (here the parameter variable p itself) is rejected (re-expected by M4.5b2 or c)
#include <cerastes.h>
#include <stddef.h>

int f(int *p)
    CERASTES_REQUIRES(cerastes_owns(p))
{
  return p == NULL;
}
