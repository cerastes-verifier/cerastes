// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/memory/cell_store_then_read_and.c
// Pins: 0039 item 2 (audit round 2, R2-B1): the value a read of *q before the store would give (0): the postcondition fails at the return, so the order of the store and the guarded read is pinned
#include <cerastes.h>
#include <stddef.h>

int *pick(int *a)
    CERASTES_REQUIRES(cerastes_live(a))
    CERASTES_ENSURES(r, cerastes_live(a) && r == a);

int f(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && *q == 0)
    CERASTES_ENSURES(r, cerastes_owns(*q) && r == 0)
{
  return ((*pick(q) = 5) == 5) && *q == 5;
}
