// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/memory/cell_store_then_read_cond.c
// Pins: 0039 item 2 (audit round 2, R2-B1): 7, the value of the other arm (the condition read before the store): the postcondition fails at the return
#include <cerastes.h>
#include <stddef.h>

int *pick(int *a)
    CERASTES_REQUIRES(cerastes_live(a))
    CERASTES_ENSURES(r, cerastes_live(a) && r == a);

int h(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && *q == 0)
    CERASTES_ENSURES(r, cerastes_owns(*q) && r == 7)
{
  return ((*pick(q) = 5) == 5) ? *q : 7;
}
