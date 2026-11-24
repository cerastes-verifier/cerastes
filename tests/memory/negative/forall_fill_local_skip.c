// expect: error uninitialized @ L20
// expect: error invariant-preserved @ L20
// mode: c23, c++20
// twin: tests/memory/forall_fill_local.c
// Pins: 0061 item 9 (S45 (c) skip_fail, question 254): the body skips the write: TWO errors at the invariant, both real: the flag line (a[i] holds no value: uninitialized) and the user clause (a[i] == i is not preserved: invariant-preserved); the owner expects both
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
    CERASTES_CHECK(i >= 0);
    i = i + 1;
  }
  return 0;
}
