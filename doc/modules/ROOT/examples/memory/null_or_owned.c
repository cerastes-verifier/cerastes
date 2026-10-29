// expect: verified
// expect: native-ok
// mode: c23, c++20
// A pointer parameter that may be NULL: the contract says what holds in each case.
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

void reset(int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
    CERASTES_ENSURES(p == NULL || (cerastes_owns(*p) && *p == 0))
{
  if (p != NULL) {
    *p = 0;
  }
}
// end::source[]
