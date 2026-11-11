// expect: error out-of-bounds @ L20
// mode: c23, c++20
// twin: tests/memory/array_loop_alias_write.c
// Pins: 0059 item 8: the loop writes a[i + 1]: at i == 2 that is a[3], outside the array: one out-of-bounds, beside the parameter place
// Pre-registered from the il-analyst's fix (report 7) of the exit 3 of sweep2/loop_alias*.c
#include <cerastes.h>

int f(int c, int *p)
    CERASTES_REQUIRES(c == 0 || cerastes_owns(*p))
    CERASTES_ENSURES(r, c == 0 || cerastes_owns(*p))
{
  int a[3] = {1, 2, 3};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (c != 0) {
      *p = i;
    }
    a[i + 1] = i;
    i++;
  }
  return a[0];
}
