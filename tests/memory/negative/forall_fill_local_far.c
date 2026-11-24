// expect: error uninitialized @ L19
// mode: c23, c++20
// twin: tests/memory/forall_fill_local.c
// Pins: 0061 item 9 (S45 (c) far_fail, question 254): the invariant one element too far: at entry a[0] is unwritten, the flag line fails (uninitialized) and, the failed line being assumed against the known unwritten element, nothing else is reported: ONE error at the invariant
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
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(filled, a, i + 1))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    a[i] = i;
    i = i + 1;
  }
  (void)a[0];
  return 0;
}
