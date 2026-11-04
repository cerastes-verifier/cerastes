// expect: verified
// expect: native-ok
// Pins: 0046 item 2: libc++'s <array> includes <stdlib.h>, which reaches the shim; a unit that calls none of its functions verifies as before
// No call of abort, exit or _Exit: the shim's contracts are loaded and nothing uses them.
#include <array>
#include <cerastes.h>

int twice(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == 2 * x)
{
  return x + x;
}
