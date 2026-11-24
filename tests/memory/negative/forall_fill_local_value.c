// expect: error invariant-preserved @ L19
// mode: c23, c++20
// twin: tests/memory/forall_fill_local.c
// Pins: 0061 item 9 (S45 (c) value_fail, question 254): a wrong value is stored (i + 1): the flag line holds, the user clause is not preserved: ONE error, the one-error twin of the skipped write
// Pre-registered from record 0061 item 9 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool filled(int k, const int *a, int m)
{
  return !(0 <= k && k < m) || a[k] == k;
}

int fill_then_read(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3];
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(filled, a, i))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    a[i] = i + 1;
    i = i + 1;
  }
  (void)a[0];
  return 0;
}
