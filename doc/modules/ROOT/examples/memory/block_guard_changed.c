// expect: error permission @ L9
// mode: c23, c++20
// The contract returns the block under a guard that reads *q on return, and does not say that *q is unchanged: the guard may differ from the one that took the block.
// tag::source[]
#include <cerastes.h>

void borrow(int *q, int *p)
    CERASTES_REQUIRES(cerastes_owns(*q) && (*q > 0 ? cerastes_owns_block(p) : true))
    CERASTES_ENSURES(cerastes_owns(*q) && (*q > 0 ? cerastes_owns_block(p) : true))
{
  if (*q > 0) {
    *p = 1;
  }
}
// end::source[]
