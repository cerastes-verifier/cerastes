// expect: error division-by-zero @ L11
// expect: error division-by-zero @ L13
// mode: c23, c++20
// The b != 0 conjunct is gone: a / b divides by zero, in the contract (L11) and in the body (L13).
// tag::source[]
#include <cerastes.h>
#include <limits.h>

int ratio(int a, int b)
    CERASTES_REQUIRES(!(a == INT_MIN && b == -1))
    CERASTES_ENSURES(r, r == a / b)
{
  return a / b;
}
// end::source[]
