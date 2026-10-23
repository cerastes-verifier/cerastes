// expect: error precondition @ L18
// mode: c23, c++20
// twin: tests/scalar/comma_call_left.c
// Pins: 0020 item 2: touch(-5) breaks the precondition of touch
#include <cerastes.h>

void touch(int v)
    CERASTES_REQUIRES(v >= 0)
{
  CERASTES_CHECK(v >= 0);
  (void)v;
}

int after(int x)
    CERASTES_REQUIRES(-5 <= x && x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  return (touch(x), x + 1);
}
