// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 6: i + 0u > 0u is true for i == -1 (the sum is computed in unsigned)
// A direct comparison of an int with an unsigned value warns with -Wsign-compare, so the test
// compares two unsigned values, one of them the result of a mixed addition.
#include <cerastes.h>

int mixed_above_zero(int i)
    CERASTES_REQUIRES(i == -1)
    CERASTES_ENSURES(r, r == 1)
{
  return i + 0u > 0u;
}
