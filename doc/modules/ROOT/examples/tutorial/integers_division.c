// expect: verified
// expect: native-ok
// mode: c23, c++20
// Division needs a nonzero divisor and no INT_MIN / -1.
// tag::source[]
#include <cerastes.h>
#include <limits.h>

int ratio(int a, int b)
    CERASTES_REQUIRES(b != 0 && !(a == INT_MIN && b == -1))
    CERASTES_ENSURES(r, r == a / b)
{
  return a / b;
}
// end::source[]
