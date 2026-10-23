// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/conv_unsigned_sites.c
// Pins: 0022 item 1: i + u with i == -1 converts i to unsigned first: a conversion error (the usual arithmetic conversions)
#include <cerastes.h>

unsigned from_add(int i, unsigned u)
    CERASTES_REQUIRES(i == -1 && u == 1u)
{
  return i + u;
}
