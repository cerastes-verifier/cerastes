// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/scalar/assign_chained.c
// Pins: 0020 item 1: x = y = z = a stores a in all three
#include <cerastes.h>

int chain(int a)
    CERASTES_REQUIRES(-715827882 <= a && a <= 715827882)
    CERASTES_ENSURES(r, r == 3 * a - 1)
{
  int x;
  int y;
  int z;
  x = y = z = a;
  return x + y + z;
}

int init_chain(int a)
    CERASTES_REQUIRES(-1073741824 <= a && a <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * a)
{
  int p;
  int q = p = a;
  return p + q;
}
