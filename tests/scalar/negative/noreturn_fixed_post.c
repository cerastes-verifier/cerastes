// expect: error postcondition @ L20
// mode: c23, c++20
// twin: doc/modules/ROOT/examples/error-messages/noreturn_fixed.c
// Pins: 0015 item 4: the ensures of the doc example is checked on the path that returns
// The test lets x == 0 through, so the function may return 0, which the ensures r > 0 forbids.
#include <stdlib.h>
#include <cerastes.h>

[[noreturn]] void fail(void)
{
  abort();
}

int check_positive(int x)
    CERASTES_ENSURES(r, r > 0)
{
  if (x < 0) {
    fail();
  }
  return x;
}
