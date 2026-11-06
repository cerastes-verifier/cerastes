// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/formal_pointee_given.c
// Pins: 0052 item 2: the same ensures(owns(*p)) with p == &g known but g not held: permission at the closing brace of the void function (its return row)
// Pre-registered from record 0052 item 2: permission at the closing brace (a void function's return row; the row may be reported at the clause instead, then set to the first run's, with a reason)
#include <cerastes.h>

static int g = 0;

void rename_cell(int *p)
    CERASTES_REQUIRES(p == &g)
    CERASTES_ENSURES(cerastes_owns(*p) && p == &g)
{
  (void)p;
}  // <- here

int get(void)
    CERASTES_REQUIRES(cerastes_reads(g))
    CERASTES_ENSURES(r, cerastes_reads(g) && r == g)
{
  return g;
}
