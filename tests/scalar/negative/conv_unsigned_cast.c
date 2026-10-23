// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/conv_unsigned_sites.c
// Pins: 0022 item 1: (unsigned)x with x == -1 is a conversion error at the cast
#include <cerastes.h>

unsigned from_cast(int x)
    CERASTES_REQUIRES(x == -1)
{
  return (unsigned)x;
}
