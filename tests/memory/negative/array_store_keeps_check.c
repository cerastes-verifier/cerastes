// expect: error check @ L13
// mode: c23, c++20
// twin: tests/memory/array_store_keeps.c
// Pins: 0059 item 4: a[0] is still 1 after the store to a[1], so a check of 2 fails
// Pre-registered from record 0059 item 4
#include <cerastes.h>

int store(void)
    CERASTES_ENSURES(r, r == 6)
{
  int a[3] = {1, 2, 3};
  a[1] = 5;
  CERASTES_CHECK(a[0] == 2);
  CERASTES_CHECK(a[1] == 5);
  CERASTES_CHECK(a[2] == 3);
  return a[0] + a[1];
}
