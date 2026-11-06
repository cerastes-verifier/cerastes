// expect: verified
// expect: native-ok
// mode: c23, c++20
// A block borrowed under a guard and returned under it: *q == old(*q) says the guard holds on return as it did on entry.
// tag::source[]
#include <cerastes.h>

void borrow(int *q, int *p)
    CERASTES_REQUIRES(cerastes_owns(*q) && (*q > 0 ? cerastes_owns_block(p) : true))
    CERASTES_ENSURES(cerastes_owns(*q) && *q == cerastes_old(*q) && (*q > 0 ? cerastes_owns_block(p) : true))
{
  if (*q > 0) {
    *p = 1;
  }
}
// end::source[]
