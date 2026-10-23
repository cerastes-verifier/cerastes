// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021: (f(), 1) + g(): the comma orders f before its right operand, but g is unsequenced with the pair; the sets form a chain
#include <cerastes.h>

int one(int x)
    CERASTES_REQUIRES(0 <= x && x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1 && r > 0)
{
  return x + 1;
}

int two(int y)
    CERASTES_REQUIRES(0 <= y && y < 2147483647)
    CERASTES_ENSURES(r, r == y + 1 && r > 0)
{
  return y + 1;
}

int test(int x, int y)
    CERASTES_REQUIRES(0 <= x && x < 2147483647 && 0 <= y && y < 2147483646)
    CERASTES_ENSURES(r, r == y + 2)
{
  return (one(x), 1) + two(y);
}
