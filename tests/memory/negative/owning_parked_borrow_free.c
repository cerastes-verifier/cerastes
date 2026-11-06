// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/owning_parked_borrow.c
// Pins: 0052 item 3 (audit S11): the owning local frees the borrowed block that the ensures hands back: permission at the closing brace
// Pre-registered from record 0052 item 3 (audit round 1, S12): permission at the closing brace
#include <cerastes.h>
#include <stdlib.h>

void park(int *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(cerastes_owns_block(p))
{
  CERASTES_OWNING int *v = p;
  free(v);
}  // <- here
