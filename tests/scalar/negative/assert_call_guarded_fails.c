// expect: error check @ L18
// mode: c23, c++20
// twin: tests/scalar/assert_call_guarded.c
// Pins: 0021 and 0015 item 6: pos(1) == 1 is not > 1, so the assert fails at a == 1
#include <assert.h>
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int test(int a)
    CERASTES_ENSURES(r, r == a)
{
  assert(a <= 0 || pos(a) > 1);
  return a;
}
