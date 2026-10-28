// expect: error check @ L14
// mode: c23, c++20
// twin: tests/memory/address_compare.c
// Pins: 0032 item 7: the addresses of two different locals are never equal
#include <cerastes.h>

int addresses(void)
    CERASTES_ENSURES(r, r == 2)
{
  int x = 1;
  int y = 2;
  int *p = &x;
  int n = 0;
  CERASTES_CHECK(&x == &y);
  if (p == &x) {
    n += 1;
  }
  if (p != &y) {
    n += 1;
  }
  return n;
}
