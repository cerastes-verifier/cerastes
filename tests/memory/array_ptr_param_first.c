// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 4 and 10 (S43 (i)): p[0] on a parameter with owns(*p) verifies: the emitted atom already says offset_of(p) < len(block_of(p))
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code: verifies
#include <cerastes.h>

int first(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == *p)
{
  return p[0];
}
