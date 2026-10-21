// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 6 and 0012 item 6: a call in an assert is verified against the callee's contract
// The callee's ensures makes the assertion true.
#include <assert.h>
#include <cerastes.h>

int twice(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1000)
    CERASTES_ENSURES(r, r == 2 * x)
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
