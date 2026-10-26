// expect: error overflow @ L7
// mode: c23, c++20
// The compiler accepts this assertion (the unsigned product wraps to 0), but the product does not fit in unsigned: cerastes reports it at the assertion.
// tag::source[]
#include <cerastes.h>

static_assert(65536u * 65536u == 0u, "the product wraps");

int bump(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}
// end::source[]
