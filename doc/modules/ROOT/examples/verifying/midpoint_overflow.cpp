// expect: error overflow @ L10
// The walkthrough with the classic mistake: lo + hi can overflow.
// tag::source[]
#include <cerastes.h>

int midpoint(int lo, int hi)
    CERASTES_REQUIRES(0 <= lo && lo <= hi)
    CERASTES_ENSURES(r, lo <= r && r <= hi)
{
  return (lo + hi) / 2;
}
// end::source[]
