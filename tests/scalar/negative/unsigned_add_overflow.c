// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_in_range.c
// Pins: 0022 item 1: UINT_MAX + 1u does not fit unsigned int: overflow, no wrap to 0
#include <cerastes.h>

unsigned add_u(unsigned a)
    CERASTES_REQUIRES(a == 4294967295u)
    CERASTES_ENSURES(r, r == 0u)
{
  return a + 1u;
}
