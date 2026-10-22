// expect: error division-by-zero @ L10
// mode: c23, c++20
// twin: tests/scalar/guard_kept.c
// Pins: 0010 item 11: `||` swapped to `&&` in a check (the guard `y == 0 ||` no longer guards the division)
#include <cerastes.h>

int by_check(int x, int y)
    CERASTES_REQUIRES(x >= 0 && y == 0)
{
  CERASTES_CHECK(y == 0 && x / y == 0);
  return x;
}
