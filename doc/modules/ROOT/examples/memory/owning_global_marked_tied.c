// expect: verified
// expect: native-ok
// mode: c23, c++20
// An owning global that is also marked ref_to_uninit: the caller's value in the block survives a call whose contract ties the global.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int *g CERASTES_OWNING CERASTES_REF_TO_UNINIT;

void touch(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g) && g == cerastes_old(g))
{
}

void use(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  if (g != NULL) {
    *g = 1;
  }
  touch();
  if (g != NULL) {
    int y = *g;
    *g = y;
  }
}
// end::source[]
