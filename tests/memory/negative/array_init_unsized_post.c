// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/memory/array_init_unsized.c
// Pins: 0059 item 3: a[3] is the fourth element, 4
// Pre-registered from record 0059 item 3
#include <cerastes.h>

int unsized(void)
    CERASTES_ENSURES(r, r == 3)
{
  int a[] = {1, 2, 3, 4};
  return a[3];
}
