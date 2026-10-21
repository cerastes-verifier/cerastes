// expect: error overflow @ L8
// mode: c23, c++20
// twin: tests/scalar/guard_kept.c
// Pins: 0010 item 11: `||` swapped to `&&` in a requires; `x + 1` is evaluated for x == 2147483647
#include <cerastes.h>

int by_requires_overflow(int x)
    CERASTES_REQUIRES(x == 2147483647 && x + 1 > 5)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}
