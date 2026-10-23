// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021: (f(), 1) + g(): the comma orders f before its right operand, but g is unsequenced with the pair; the sets form a chain
#include <cerastes.h>

int one(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}

int two(int y)
    CERASTES_REQUIRES(y < 2147483647)
    CERASTES_ENSURES(r, r == y + 1)
{
  return y + 1;
}

int test(int x, int y)
    CERASTES_REQUIRES(x < 2147483647 && y < 2147483646)
    CERASTES_ENSURES(r, r == y + 2)
{
  return (one(x), 1) + two(y);
}

int two_edge(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return two(2147483646);
}
