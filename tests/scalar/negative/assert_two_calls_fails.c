// expect: error check @ L26
// mode: c23, c++20
// twin: tests/scalar/assert_two_calls.c
// Pins: 0021 and 0015 item 6: at x == y == 0 the sum is 2, which is not > 2
#include <assert.h>
#include <cerastes.h>

int succ(int x)
    CERASTES_REQUIRES(x <= 1000)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}

int pred(int y)
    CERASTES_REQUIRES(y <= 1000)
    CERASTES_ENSURES(r, r == y + 1)
{
  return y + 1;
}

int test(int x, int y)
    CERASTES_REQUIRES(0 <= x && x <= 1000 && 0 <= y && y <= 1000)
    CERASTES_ENSURES(r, r == x + y)
{
  assert(succ(x) + pred(y) > 2);
  return x + y;
}
