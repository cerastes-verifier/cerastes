// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 10: a loop that stores through pk[0] where pk = p (a parameter's pointer, not reassigned in the loop)
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code: verifies
#include <cerastes.h>

void fill(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int *pk = p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    pk[0] = i;
    i = i + 1;
  }
}
