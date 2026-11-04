// expect: error check @ L14
// mode: c23, c++20
// The check says x <= 99, which does not hold when x is 100.
// Derived from first_proof_check.c.
// tag::source[]
#include <cerastes.h>

int clamp_to_100(int x)
    CERASTES_ENSURES(r, r <= 100)
{
  if (x > 100) {
    x = 100;
  }
  CERASTES_CHECK(x <= 99);
  return x;
}
// end::source[]
