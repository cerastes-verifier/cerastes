// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 8 (0038 item 3): an array place beside a parameter place that may not be owned (c == 0 || owns(*p)): the parameter's value and liveness conjuncts say its block is not the array's, so the loop's writes to the array and through p do not clash
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
    if (c != 0) {
      *p = i;
    }
    a[i] = i;
    i++;
  }
  return a[0];
}
