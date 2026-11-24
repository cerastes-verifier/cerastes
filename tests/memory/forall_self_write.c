// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 9 (S45 (c) self_write): the sibling of array_loop_self_write_false_alarm.c: the loop stores a[i] = a[i], a write reaches the array so its values are not kept, but the quantified invariant a[k] == k + 1 states them and r == 3 follows
// Pre-registered from record 0061 item 9 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool kept_at(int k, const int *a, int m)
{
  return !(0 <= k && k < m) || a[k] == k + 1;
}

int f(void)
    CERASTES_ENSURES(r, r == 3)
{
  int a[3] = {1, 2, 3};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(kept_at, a, 3))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    a[i] = a[i];
    i = i + 1;
  }
  return a[2];
}
