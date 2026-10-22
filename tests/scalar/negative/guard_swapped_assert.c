// expect: error division-by-zero @ L11
// mode: c23, c++20
// twin: tests/scalar/guard_kept.c
// Pins: 0010 item 11 and 0015: `||` swapped to `&&` in an assert; the division is checked in the state
#include <assert.h>
#include <cerastes.h>

int by_assert(int x, int y)
    CERASTES_REQUIRES(x >= 0 && y == 0)
{
  assert(y == 0 && x / y == 0);
  return x;
}
