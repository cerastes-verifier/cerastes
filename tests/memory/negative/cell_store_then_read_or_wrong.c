// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/memory/cell_store_then_read_or.c
// Pins: 0039 item 2 (audit round 2, R2-B1): the value a read of *q before the store would give (0): the postcondition fails at the return
#include <cerastes.h>
#include <stddef.h>

int tick(void)
    CERASTES_ENSURES(r, r == 0);

int f(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p == 0)
    CERASTES_REQUIRES(q == p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 0)
{
  return ((*(tick(), p) = 5) != 5) || *q == 5;
}
