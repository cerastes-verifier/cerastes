// expect: error check @ L12
// mode: c23, c++20
// twin: tests/memory/array_sizeof_value.c
// Pins: 0059 item 11: the array has three elements, so sizeof(a) == 2 * sizeof(int) fails
// Pre-registered from record 0059 item 11 (Part E), before the code
#include <cerastes.h>

int size(void)
    CERASTES_ENSURES(r, r == 12)
{
  int a[3] = {1, 2, 3};
  CERASTES_CHECK(sizeof(a) == 2 * sizeof(int));
  return (int)sizeof(a) + a[0] - 1;
}
