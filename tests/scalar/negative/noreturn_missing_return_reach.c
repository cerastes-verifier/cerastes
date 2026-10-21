// expect: error missing-return @ L23
// mode: c23, c++20
// twin: tests/scalar/noreturn_missing_return.c
// Pins: 0015 item 4: the call of a [[noreturn]] function ends only the path it is on
// For -5 <= x <= 0 no call is made and the function falls off its end.
#include <stdlib.h>
#include <cerastes.h>

[[noreturn]] void fail(void)
{
  abort();
}

int must_be_positive(int x)
    CERASTES_ENSURES(r, r == x && x > 0)
{
  if (x > 0) {
    return x;
  }
  if (x < -5) {
    fail();
  }
}
