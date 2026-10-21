// expect: error check @ L12
// mode: c23, c++20
// twin: tests/scalar/assert_holds.c
// Pins: 0015 item 6: assert(0) always fails: the path that reaches it has a check error
// The constant condition is the user's text, so the check is reported at the assert.
#include <assert.h>
#include <cerastes.h>

int never(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  assert(0);
  return x;
}
