// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_in_range.c
// Pins: 0022 item 1: -1u does not fit unsigned int: overflow, no wrap to UINT_MAX
#include <cerastes.h>

unsigned neg_u(unsigned a)
    CERASTES_REQUIRES(a == 1u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return -a;
}
