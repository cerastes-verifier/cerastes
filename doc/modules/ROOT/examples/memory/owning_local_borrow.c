// expect: verified
// expect: native-ok
// mode: c23, c++20
// A function borrows a block (owns_block in both lists) and parks it in an owning local: the borrowed block counts as an owner, so the local's exit holds.
// tag::source[]
#include <cerastes.h>

void park(int *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(cerastes_owns_block(p))
{
  CERASTES_OWNING int *v = p;
  *v = 1;
}
// end::source[]
