// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 12: a folded constant is checked as written; this one is exact
// EDG folds 2147483646 + 1 and keeps the expression, which the encoder encodes.
#include <cerastes.h>

int top(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return 2147483646 + 1;
}
