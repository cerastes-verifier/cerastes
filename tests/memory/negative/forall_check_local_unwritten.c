// expect: error uninitialized @ L21
// mode: c23, c++20
// twin: tests/memory/forall_check_local.c
// Pins: 0061 item 10 (S45 (f) check_unwritten_fail): a[2] is unwritten: the flag row fails (uninitialized), alone (the assumed flags contradict the known unwritten element, so the fact row is not reported)
// Pre-registered from record 0061 item 10 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int check_it(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_UNINIT int a[4];
  a[0] = 1;
  a[1] = 2;
  a[3] = 4;
  CERASTES_CHECK(cerastes_forall(nonneg, a, 4));
  (void)a[0];
  return 0;
}
