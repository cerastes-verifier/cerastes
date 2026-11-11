// expect: error invariant-preserved @ L16
// mode: c23, c++20
// twin: tests/memory/array_loop_sum.c
// Pins: 0059 item 8: the loop writes the array first (a[i] = 7), so a write reaches it: its values are not kept and s <= 3 * i is not preserved
// Pre-registered from record 0059 item 8 (Part C), before the code
#include <cerastes.h>

int sum(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 9)
{
  int a[3] = {1, 2, 3};
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(0 <= s)
  CERASTES_INVARIANT(s <= 3 * i)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    a[i] = 7;
    s = s + a[i];
    i = i + 1;
  }
  return s;
}
