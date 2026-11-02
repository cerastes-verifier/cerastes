// expect: error permission @ L9
// mode: c23, c++20
// twin: tests/memory/init_alias_unmarked.c
// Pins: 0042 item 6 (audit S4): a pre-state read through an alias in a requires: the marked atom frames nothing, so permission at the clause
#include <cerastes.h>

void f(int *p CERASTES_MUST_INIT, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && q == p)
    CERASTES_REQUIRES(*q > 0)  // <- here
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)q;
}
