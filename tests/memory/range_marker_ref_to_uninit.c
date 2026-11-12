// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 12 (S44 (h) ref_keeps_flag): a ref_to_uninit callee over owns_range(p, 3) (no flags on entry, old(flag) ==> flag on return); a caller passes an initialized array, and the elements stay initialized after the call
// Pre-registered from record 0060 item 12 (Part E), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

void wr(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns_range(p, 3))
    CERASTES_ENSURES(cerastes_owns_range(p, 3))
{
  p[0] = 7;
}

int keeps(void)
{
  int a[3] = {1, 2, 3};
  wr(a);
  return a[1];
}
