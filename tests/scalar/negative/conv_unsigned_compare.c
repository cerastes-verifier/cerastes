// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/conv_unsigned_sites.c
// Pins: 0022 item 1: i + 0u > 0u with i == -1 was true by wrapping; now a conversion error at the sum
#include <cerastes.h>

int above_zero(int i)
    CERASTES_REQUIRES(i == -1)
{
  return i + 0u > 0u;
}
