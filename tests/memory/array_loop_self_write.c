// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 8: a loop that stores a[i] = a[i] beside a conditionally owned parameter: a write reaches the array, so the program says only what the loop keeps (the parameter's ownership); the stronger postcondition is the pinned false alarm array_loop_self_write_false_alarm.c
// Pre-registered from the il-analyst's fix (report 7) of the exit 3 of sweep2/loop_alias*.c: verifies
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
    a[i] = a[i];
    i++;
  }
  if (c != 0) {
    *p = 0;
  }
  return a[2];
}
