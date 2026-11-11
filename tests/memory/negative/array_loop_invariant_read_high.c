// expect: error out-of-bounds @ L14
// mode: c23, c++20
// twin: tests/memory/array_loop_invariant_read.c
// Pins: 0059 item 8: the invariant reads a[i] with i up to 3: out-of-bounds at the invariant clause ("this loop invariant reads a[e], whose index may be outside a")
// Pre-registered from record 0059 item 8 (Part C), before the code
#include <cerastes.h>

int first(void)
    CERASTES_ENSURES(r, r >= 1)
{
  int a[3] = {1, 2, 3};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(a[i] >= 1)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = i + 1;
  }
  return a[0];
}
