// expect: verified
// expect: native-ok
// Pins: 0046 item 4: std::_Exit and std::abort through <cstdlib> are vlibc's `_Exit` and `abort` (C linkage, the trusted `ensures(false)`)
// Only x <= 5 reaches the return.
#include <cstdlib>
#include <cerastes.h>

int pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && x <= 5)
{
  if (x > 7) {
    std::_Exit(1);
  }
  if (x > 5) {
    std::abort();
  }
  return x;
}
