// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 items 2 and 6: h(f(x), y) + 100 / g(z) with h's requires reading only y; the exact sets {f} and {g} are not a chain, so the base sets are used (step 2), and with the caller's requires on y the program verifies
#include <cerastes.h>

int f(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int g(int z)
    CERASTES_ENSURES(r, r == 1)
{
  return 1 + (z - z);
}

int h(int a, int b)
    CERASTES_REQUIRES(0 < b)
    CERASTES_ENSURES(r, r == b)
{
  return b + (a - a);
}

int test(int x, int y, int z)
    CERASTES_REQUIRES(0 < y && y <= 2147483547)
    CERASTES_ENSURES(r, r == y + 100)
{
  return h(f(x), y) + 100 / g(z);
}
