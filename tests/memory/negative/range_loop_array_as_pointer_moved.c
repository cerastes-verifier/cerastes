// expect: error invariant-preserved @ L13
// mode: c23, c++20
// twin: tests/memory/range_loop_array_as_pointer.c
// Pins: 0060 item 11 (A3's twin): the body moves q to &a[1]: q == a is not preserved: invariant-preserved at the invariant clause
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>

int walk(void)
{
  int a[3] = {1, 2, 3};
  int *q = &a[0];
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && q == a)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    q = &a[1];
    i += 1;
  }
  return *q;
}
