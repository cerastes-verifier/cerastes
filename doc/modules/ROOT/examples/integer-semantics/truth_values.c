// expect: verified
// expect: native-ok
// mode: c23, c++20
// In C a condition is true when it is not zero, and a comparison is an int; the same source in pass:[C++].
// tag::source[]
#include <cerastes.h>

int is_zero(int x)
    CERASTES_ENSURES(r, r == (x == 0))
{
  if (x) {
    return 0;
  }
  return 1;
}
// end::source[]
