// expect: verified
// expect: native-ok
// mode: c23, c++20
// The same function, fixed: *p is written on every path.
// tag::source[]
#include <cerastes.h>

void init(int *p CERASTES_MUST_INIT, int c)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  if (c != 0) {
    *p = 0;
  } else {
    *p = 1;
  }
}
// end::source[]
