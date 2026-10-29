// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 items 3 and 4: a parameter with owns(*p) is an automatic place of the loop (the function never assigns p): written in the loop, its value stated in the invariant
#include <cerastes.h>

void fill(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == n)
{
  int i = 0;
  *p = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && *p == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p += 1;
    i += 1;
  }
}
