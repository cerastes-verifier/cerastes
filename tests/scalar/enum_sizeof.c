// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0018 item 10: an enumerator initializer that is a sizeof expression is a constant (4 for int) in both modes
#include <cerastes.h>

enum { SZ = sizeof(int) };

int bytes(void)
    CERASTES_ENSURES(r, r == 4)
{
  return SZ;
}

int twice(int x)
    CERASTES_REQUIRES(x <= 2147483643)
    CERASTES_ENSURES(r, r == x + 4)
{
  return x + SZ;
}

int twice_edge(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return twice(2147483643);
}
