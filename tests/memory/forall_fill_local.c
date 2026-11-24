// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 9 (S45 (c) fill_local; Part C): the sibling of array_loop_fill_then_read_false_alarm.c: a fill loop over `int a[3];` declared without an initializer states with a quantified invariant that the elements below i are written and equal their index, so the read after the loop is initialized and a[1] == 1
// Pre-registered from record 0061 item 9 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool filled(int k, const int *a, int m)
{
  return !(0 <= k && k < m) || a[k] == k;
}

int fill_then_read(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[3];
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(filled, a, i))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    a[i] = i;
    i = i + 1;
  }
  return a[1];
}
