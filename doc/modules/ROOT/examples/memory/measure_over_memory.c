// expect: verified
// expect: native-ok
// mode: c23, c++20
// A termination measure that reads the cell the function owns.
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

int down(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p >= 0 && *p < 100)
    CERASTES_DECREASES(*p)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  if (*p == 0) {
    return 0;
  }
  *p -= 1;
  return down(p);
}
// end::source[]
