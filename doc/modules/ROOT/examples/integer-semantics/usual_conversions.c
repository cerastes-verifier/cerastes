// expect: verified
// expect: native-ok
// mode: c23, c++20
// int meets unsigned: the int converts to unsigned, which is exact while it is not negative.
// tag::source[]
#include <cerastes.h>

int sum_is_positive(int i)
    CERASTES_REQUIRES(i >= 0)
    CERASTES_ENSURES(r, r == (i > 0))
{
  return i + 0u > 0u;
}
// end::source[]
