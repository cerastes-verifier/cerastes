// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 and 0015 item 6: an assert whose condition holds two unsequenced calls; succ(x) + pred(y) >= 2
#include <assert.h>
#include <cerastes.h>

int succ(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1000)
    CERASTES_ENSURES(r, r == x + 1 && 1 <= r && r <= 1001)
{
  return x + 1;
}

int pred(int y)
    CERASTES_REQUIRES(0 <= y && y <= 1000)
    CERASTES_ENSURES(r, r == y + 1 && 1 <= r && r <= 1001)
{
  return y + 1;
}

int test(int x, int y)
    CERASTES_REQUIRES(0 <= x && x <= 1000 && 0 <= y && y <= 1000)
    CERASTES_ENSURES(r, r == x + y)
{
  assert(succ(x) + pred(y) > 1);
  return x + y;
}
