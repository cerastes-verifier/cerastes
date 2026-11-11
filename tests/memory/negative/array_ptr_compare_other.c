// expect: error check @ L13
// mode: c23, c++20
// twin: tests/memory/array_ptr_compare.c
// Pins: 0059 item 10: p = &a[1] is not the address of a[0]: the check a == p fails
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code
#include <cerastes.h>

int same(void)
    CERASTES_ENSURES(r, r == 2)
{
  int a[3] = {1, 2, 3};
  int *p = &a[1];
  CERASTES_CHECK(a == p);
  return *p;
}
