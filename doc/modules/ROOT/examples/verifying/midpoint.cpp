// expect: verified
// expect: native-ok
// The walkthrough function: the midpoint of two ordered non-negative ints.
// tag::source[]
#include <cerastes.h>

int midpoint(int lo, int hi)
    CERASTES_REQUIRES(0 <= lo && lo <= hi)
    CERASTES_ENSURES(r, lo <= r && r <= hi)
{
  return lo + (hi - lo) / 2;
}
// end::source[]
