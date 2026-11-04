// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/ptrptr_owns_inner.c
// Pins: 0045 item 1: owns(**r) through the result: M4.5d2b
#include <cerastes.h>

int **f(void)
    CERASTES_ENSURES(r, cerastes_owns(**r))  // <- here
{
  return 0;
}
