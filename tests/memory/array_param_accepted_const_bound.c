// expect: verified
// expect: native-ok
// mode: c23
// c-only: `int a[const 3]` is C only (a qualifier inside the brackets)
// Pins: 0060 item 15 (Step 0 M: silent; C only): an `int a[const 3]` parameter is accepted (the bound is ignored)
// Pre-registered from record 0060 item 15 (Part E), before the code: verifies
#include <cerastes.h>

int first(int a[const 3])
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(r, cerastes_owns(*a))
{
  return a[0];
}
