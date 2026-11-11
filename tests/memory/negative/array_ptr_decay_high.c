// expect: error out-of-bounds @ L13
// mode: c23, c++20
// twin: tests/memory/array_ptr_decay.c
// Pins: 0059 items 4 and 10: p points at a[0] of a three-element object, so p[3] is outside it: out-of-bounds at the store ("writes p[i] outside the object p points into")
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code
#include <cerastes.h>

int decay(void)
    CERASTES_ENSURES(r, r == 7)
{
  int a[3] = {1, 2, 3};
  int *p = a;
  p[3] = 7;
  return a[2];
}
