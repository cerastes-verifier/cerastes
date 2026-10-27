// expect: error check @ L11
// mode: c23, c++20
// twin: tests/scalar/void_conditional.c
// Pins: 0025 item 3: an assert in an arm is a check on that path: it fails when c holds and x does not
#include <assert.h>
#include <cerastes.h>

int assert_arm(int c, int x)
    CERASTES_ENSURES(r, r == c)
{
  c ? assert(x > 0) : (void)0;
  return c;
}
