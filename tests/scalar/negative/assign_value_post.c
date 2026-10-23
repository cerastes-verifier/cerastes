// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/scalar/assign_value.c
// Pins: 0020 item 1: the value of (y = a) + 1 is a + 1, not a
#include <cerastes.h>

int id(int v)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int chain(int a)
    CERASTES_REQUIRES(-1073741824 <= a && a <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * a)
{
  int y;
  int z = (y = a) + 1;
  return z + y;
}

int via_call(int a)
    CERASTES_REQUIRES(-1073741824 <= a && a <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * a)
{
  int y;
  int z = id(y = a);
  return z + y;
}
