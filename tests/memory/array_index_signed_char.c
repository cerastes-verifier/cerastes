// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 4 (audit round 1, item 6): a signed char index is accepted (no compiler warns); its value is in [-128, 127], so the bound row needs 0 <= c < 3
// Pre-registered from record 0059 item 4 as updated by audit round 1: verifies
#include <cerastes.h>

int pick(signed char c)
    CERASTES_REQUIRES(0 <= c && c < 3)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {0};
  return a[c];
}
