// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/live_result.c
// Pins: 0037 item 15, flipped by record 0052 item 2 (approved Flip, M4.5d2b1): an atom on *result is accepted now; this callee holds only live(p), so owns(*r) is given and not held: permission at the return (the rejection it pinned is lifted)
#include <cerastes.h>
#include <stddef.h>

int *same(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_live(p) && cerastes_owns(*r))
{
  return p;
}
