// expect: error postcondition @ L14
// mode: c23, c++20
// The last line returns a instead of b, so the result can be smaller than b.
// Derived from first_proof_max.c.
// tag::source[]
#include <cerastes.h>

int max_of(int a, int b)
    CERASTES_ENSURES(r, r >= a && r >= b && (r == a || r == b))
{
  if (a > b) {
    return a;
  }
  return a;
}
// end::source[]
