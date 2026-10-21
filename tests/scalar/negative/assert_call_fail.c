// expect: error check @ L20
// mode: c23, c++20
// twin: tests/scalar/assert_call.c
// Pins: 0015 item 6 and 0012 item 6: the assertion uses the callee's contract, not its body
// twice() promises only r >= x, so r == 2 * x is not known at the assert (its body would satisfy it).
#include <assert.h>
#include <cerastes.h>

int twice(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1000)
    CERASTES_ENSURES(r, r >= x)
{
  return x * 2;
}

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1000)
    CERASTES_ENSURES(r, r == x)
{
  assert(twice(x) == 2 * x);
  return x;
}
