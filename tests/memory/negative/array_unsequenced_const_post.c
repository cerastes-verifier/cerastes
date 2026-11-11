// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/array_unsequenced_const.c
// Pins: 0059 item 9: a[0] gets the old a[1], 2, and a[1] becomes 3: 23
// Pre-registered from record 0059 item 9
#include <cerastes.h>

int shift(void)
    CERASTES_ENSURES(r, r == 22)
{
  int a[3] = {1, 2, 3};
  a[0] = a[1]++;
  return a[0] * 10 + a[1];
}
