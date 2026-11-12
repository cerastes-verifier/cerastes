// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 15 (Step 0 M: silent in both compilers and modes): an `int a[0]` parameter is accepted (the bound is ignored)
// Pre-registered from record 0060 item 15 (Part E), before the code: verifies
#include <cerastes.h>

int first(int a[0])
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(r, cerastes_owns(*a))
{
  return a[0];
}
