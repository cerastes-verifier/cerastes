// expect: verified
// expect: native-ok
// mode: c23, c++20
// int + unsigned is computed in unsigned: the sum -1 + 0u is large.
// tag::source[]
#include <cerastes.h>

int sum_is_positive(int i)
    CERASTES_REQUIRES(i == -1)
    CERASTES_ENSURES(r, r == 1)
{
  return i + 0u > 0u;
}
// end::source[]
