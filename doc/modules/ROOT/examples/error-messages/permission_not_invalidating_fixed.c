// expect: verified
// expect: native-ok
// mode: c23, c++20
// The requires takes *p under the same condition that the ensures returns it.
// tag::source[]
#include <cerastes.h>

void maybe(int *p CERASTES_NOT_INVALIDATING, int c)
    CERASTES_REQUIRES(c != 0 ? cerastes_owns(*p) : 1)
    CERASTES_ENSURES(c != 0 ? cerastes_owns(*p) : 1);

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 1;
  maybe(&x, 1);
  maybe(&x, 0);
  return 0;
}
// end::source[]
