// expect: verified
// expect: native-ok
// mode: c23, c++20
// A forward goto leaves a loop; after the label, each way of getting there is proved separately.
// tag::source[]
#include <cerastes.h>

// The integer square root of n.
int root(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_ENSURES(r, 0 <= r && r <= 31 && r * r <= n && n < (r + 1) * (r + 1))
{
  int r = 0;
  CERASTES_INVARIANT(0 <= r && r <= 31 && r * r <= n)
  CERASTES_DECREASES(31 - r)
  while (r < 31) {
    if ((r + 1) * (r + 1) > n) {
      goto found;
    }
    r += 1;
  }
found:
  return r;
}
// end::source[]
