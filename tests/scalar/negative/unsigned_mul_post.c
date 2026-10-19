// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_mul.c
// Pins: 0010 item 4: 65536u * 65536u is 0, not nonzero
#include <cerastes.h>

unsigned mul_u(unsigned a)
    CERASTES_REQUIRES(a == 65536u)
    CERASTES_ENSURES(r, r != 0u)
{
  return a * 65536u;
}
