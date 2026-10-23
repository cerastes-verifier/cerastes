// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_in_range.c
// Pins: 0022 item 1: 65536u * 65536u does not fit unsigned int: overflow, no wrap to 0
#include <cerastes.h>

unsigned mul_u(unsigned a)
    CERASTES_REQUIRES(a == 65536u)
    CERASTES_ENSURES(r, r == 0u)
{
  return a * 65536u;
}
