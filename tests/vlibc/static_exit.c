// expect: verified
// expect: native-ok
// c-only: in C++ the global-namespace name exit is reserved whatever its linkage (decision 0046 item 6), so a static exit is refused there
// Pins: 0046 item 6 (audit round 2, note 2; C23 7.1.3): a `static` function named exit in C has internal linkage, so it is not the library's: an ordinary function, whose call returns
// The call returns, so only the postcondition that holds on both paths is claimed.
#include <cerastes.h>

static void exit(int status)
{
  (void) status;
}

int pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x)
{
  if (x > 5) {
    exit(1);
  }
  return x;
}
