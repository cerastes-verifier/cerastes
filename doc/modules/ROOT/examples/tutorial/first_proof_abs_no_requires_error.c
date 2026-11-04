// expect: error overflow @ L10
// expect: error overflow @ L13
// mode: c23, c++20
// The precondition is gone: -x overflows when x is INT_MIN, in the contract (L10) and in the body (L13).
// tag::source[]
#include <cerastes.h>
#include <limits.h>

int abs_value(int x)
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
