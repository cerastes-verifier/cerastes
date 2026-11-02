// expect: error permission @ L9
// mode: c23, c++20
// twin: tests/memory/init_alias_unmarked.c
// Pins: 0042 item 6 (audit S4): old(*q) with q == p reads the pointee on entry: the marked atom frames nothing, so permission at the clause
#include <cerastes.h>

void f(int *p CERASTES_MUST_INIT, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && q == p)
    CERASTES_ENSURES(cerastes_owns(*p) && q == p && cerastes_old(*q) == 0)  // <- here
{
  *p = 0;
}
