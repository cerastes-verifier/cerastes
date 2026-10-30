// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/memory/cell_store_then_postfix_and.c
// Pins: 0039 item 2 (audit round 2, R2-B1): the value the unordered reading would give (5, the increment lost): the postcondition fails at the closing brace
#include <cerastes.h>
#include <stddef.h>

int *pick(int *a)
    CERASTES_REQUIRES(cerastes_live(a))
    CERASTES_ENSURES(r, cerastes_live(a) && r == a);

void g(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && *q == 0)
    CERASTES_ENSURES(cerastes_owns(*q) && *q == 5)
{
  int s = ((*pick(q) = 5) == 5) && (*q)++;
  (void)s;
}
