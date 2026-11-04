// expect: error postcondition @ L26
// mode: c23, c++20
// The contract of clamp promises only the range; clamp still verifies, but its caller cannot prove r == 5.
// Derived from calls_clamp.c.
// tag::source[]
#include <cerastes.h>

int clamp(int x, int lo, int hi)
    CERASTES_REQUIRES(lo <= hi)
    CERASTES_ENSURES(r, lo <= r && r <= hi)
{
  if (x < lo) {
    return lo;
  }
  if (x > hi) {
    return hi;
  }
  return x;
}

int use_clamp(void)
    CERASTES_ENSURES(r, r == 5)
{
  int a = clamp(5, 0, 10);
  int b = clamp(-3, 0, 10);
  return a - b;
}
// end::source[]
