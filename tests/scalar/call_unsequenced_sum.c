// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 (the chain rule): two calls that are not nested in each other are accepted; each check goes right after the calls it depends on
#include <cerastes.h>

int bounded(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r == (x > 1000 ? 1000 : x) && r >= 0)
{
  return x > 1000 ? 1000 : x;
}

int sum_two(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r == (a > 1000 ? 1000 : a) + (b > 1000 ? 1000 : b))
{
  return bounded(a) + bounded(b);
}
