// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 6: recursion is verified against the contract (partial correctness)
#include <cerastes.h>

int count_down(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0)
{
  if (n == 0) {
    return 0;
  }
  return count_down(n - 1);
}
