// expect: verified
// expect: native-ok
// mode: c23, c++20
// A precondition rules out the one overflowing input; the caller relies on the postcondition.
// tag::source[]
#include <cerastes.h>
#include <limits.h>

int abs_value(int x)
    CERASTES_REQUIRES(x != INT_MIN)
    CERASTES_ENSURES(r, r >= 0 && (r == x || r == -x))
{
  if (x < 0) {
    return -x;
  }
  return x;
}

int distance_from_zero(void)
    CERASTES_ENSURES(r, r == 5)
{
  return abs_value(-5);
}
// end::source[]
