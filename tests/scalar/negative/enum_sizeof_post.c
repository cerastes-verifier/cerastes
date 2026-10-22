// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/enum_sizeof.c
// Pins: 0018 item 10: SZ is sizeof(int), which is 4, so r == 5 does not hold
#include <cerastes.h>

enum { SZ = sizeof(int) };

int bytes(void)
    CERASTES_ENSURES(r, r == 5)
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
