// expect: error division-by-zero @ L13
// mode: c23, c++20
// twin: tests/scalar/assert_obligation.c
// Pins: 0015 item 6 and 0010: the division inside the assert is checked, at the assert's line
// y may be 0 (the requires allows it); the assertion x / y > 0 is not the first problem.
#include <assert.h>
#include <cerastes.h>

int ratio(int x, int y)
    CERASTES_REQUIRES(0 <= y && y <= x)
    CERASTES_ENSURES(r, r > 0)
{
  assert(x / y > 0);
  return x / y;
}
