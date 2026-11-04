// expect: error precondition @ L22
// mode: c23, c++20
// The caller passes INT_MIN, which the precondition excludes.
// Derived from first_proof_abs.c.
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
    CERASTES_ENSURES(r, r >= 0)
{
  return abs_value(INT_MIN);
}
// end::source[]
