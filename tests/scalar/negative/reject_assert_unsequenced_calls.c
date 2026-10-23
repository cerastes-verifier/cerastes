// expect: rejected cerastes-unsequenced_calls @ L22
// mode: c23, c++20
// twin: tests/scalar/assert_two_calls.c
// Pins: 0021 and 0015 item 6: the condition of an assert is code: two divisions that depend on different calls are rejected like anywhere else
#include <assert.h>
#include <cerastes.h>

int f(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int g(int y)
    CERASTES_ENSURES(r, r == y)
{
  return y;
}

int test(int x, int y)
{
  assert(100 / f(x) + 100 / g(y) > 0);
  return x + y;
}
