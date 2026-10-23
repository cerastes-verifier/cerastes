// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/unsigned_compound_mixed.c
// Pins: 0022 item 1: u += 1L at UINT_MAX is 4294967296 in long; converting it back does not fit unsigned int
#include <cerastes.h>

unsigned add_long(unsigned u)
    CERASTES_REQUIRES(u == 4294967295u)
{
  u += 1L;
  return u;
}
