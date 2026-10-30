// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 3 (S28 F3, audit B4): a return inside a loop that names no memory, in a function whose contract owns *p: the loop holds the place so the return's ensures check passes (b1b reported a false permission error here)
#include <cerastes.h>

int find_five(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && 0 <= r && r <= n)
{
  (void)p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 5) {
      return i;
    }
    i += 1;
  }
  return n;
}
