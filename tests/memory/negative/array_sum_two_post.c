// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 items 3 and 4: the elements hold 1 and 3, so r == 5 fails at the return
// Pre-registered from record 0059 item 3 and 4
#include <cerastes.h>

int sum_ends(void)
    CERASTES_ENSURES(r, r == 5)
{
  int a[3] = {1, 2, 3};
  return a[0] + a[2];
}
