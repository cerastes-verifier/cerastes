// expect: error postcondition @ L23
// mode: c23, c++20
// twin: tests/memory/array_loop_self_write.c
// Pins: 0059 item 8 (\"What e1a cannot state\"): the loop stores a[i] = a[i], a write reaches the array, so its values are not kept and r == 3 (a[2]) cannot be proven: a pinned false alarm, M4.5e1c's quantified value invariants give it its verifying twin
// Pre-registered from the il-analyst's fix (report 7) of the exit 3 of sweep2/loop_alias*.c
#include <cerastes.h>

int f(int c, int *p)
    CERASTES_REQUIRES(c == 0 || cerastes_owns(*p))
    CERASTES_ENSURES(r, r == 3 && (c == 0 || cerastes_owns(*p)))
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
