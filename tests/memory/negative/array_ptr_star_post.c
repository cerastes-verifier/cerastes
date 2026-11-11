// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/array_ptr_star.c
// Pins: 0059 item 10: *a was written, so a[0] is 5, not 1
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code
#include <cerastes.h>

int star(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[3] = {1, 2, 3};
  *a = 5;
  return a[0];
}
