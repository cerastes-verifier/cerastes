// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 4 (audit round 1, item 6): an unsigned char index is accepted (no compiler warns) and is a value in [0, 255]: the bound row needs c < 3
// Pre-registered from record 0059 item 4 as updated by audit round 1: verifies
#include <cerastes.h>

int pick(unsigned char c)
    CERASTES_REQUIRES(c < 3)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {0};
  return a[c];
}
