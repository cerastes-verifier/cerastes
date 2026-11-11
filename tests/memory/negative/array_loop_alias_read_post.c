// expect: error postcondition @ L24
// mode: c23, c++20
// twin: tests/memory/array_loop_alias_read.c
// Pins: 0059 item 8: the sum is 6, not 7
// Pre-registered from the il-analyst's fix (report 7) of the exit 3 of sweep2/loop_alias*.c
#include <cerastes.h>

int f(int c, int *p)
    CERASTES_REQUIRES(c == 0 || cerastes_owns(*p))
    CERASTES_ENSURES(r, r == 7 && (c == 0 || cerastes_owns(*p)))
{
  int a[3] = {1, 2, 3};
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && s == (i == 0 ? 0 : i == 1 ? 1 : i == 2 ? 3 : 6))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s += a[i];
    i++;
  }
  if (c != 0) {
    *p = s;
  }
  return s;
}
