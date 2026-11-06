// expect: error permission @ L9
// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/ptrptr_owns_inner.c
// Pins: 0045 item 1, flipped by record 0052 item 2 (approved Flip, M4.5d2b1): owns(**r) with no atom on *r before it is a framing failure at the atom (permission, the contract reads *r without owning it) and the return does not hold **r (permission at the return); the rejection it pinned is lifted
#include <cerastes.h>

int **f(void)
    CERASTES_ENSURES(r, cerastes_owns(**r))  // <- here
{
  return 0;
}
