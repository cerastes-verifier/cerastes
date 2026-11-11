// expect: error permission @ L9
// mode: c23, c++20
// The contract takes *p whatever c is and returns it only where c is not zero, which not_invalidating forbids.
// tag::source[]
#include <cerastes.h>

void maybe(int *p CERASTES_NOT_INVALIDATING, int c)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(c != 0 ? cerastes_owns(*p) : 1);

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 1;
  maybe(&x, 1);
  return 0;
}
// end::source[]
