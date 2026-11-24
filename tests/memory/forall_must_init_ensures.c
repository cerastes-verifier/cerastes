// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 4 (a read through a marked parameter; Step 0 R1): a must_init parameter's ensures reads its elements under the binder (the flags are promised there), and the caller's uninitialized array is initialized after the call
// Pre-registered from record 0061 item 4 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool zero_at(int i, const int *p, int n)
{
  return !(0 <= i && i < n) || p[i] == 0;
}

void zero2(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns_range(p, 2))
    CERASTES_ENSURES(cerastes_owns_range(p, 2) && cerastes_forall(zero_at, p, 2))
{
  p[0] = 0;
  p[1] = 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_UNINIT int a[2];
  zero2(a);
  return a[1];
}
