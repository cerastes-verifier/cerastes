// expect: verified
// expect: native-ok
// mode: c23, c++20
// A narrowing conversion is safe under a range precondition.
// tag::source[]
#include <cerastes.h>
#include <limits.h>

short to_short(int x)
    CERASTES_REQUIRES(SHRT_MIN <= x && x <= SHRT_MAX)
    CERASTES_ENSURES(r, r == x)
{
  return (short)x;
}
// end::source[]
