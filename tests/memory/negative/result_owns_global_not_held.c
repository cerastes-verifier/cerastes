// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/result_owns_global_cell.c
// Pins: 0052 item 2: the callee returns owns(*r) of g without holding g (no requires owns(g)): not covered and nothing to give, so permission at the return (the clause is reported once, at the return row)
// Pre-registered from record 0052 item 2: permission at the return statement
#include <cerastes.h>

static int g = 3;

int *take_cell(void)
    CERASTES_ENSURES(r, cerastes_owns(*r) && r == &g)
{
  return &g;  // <- here
}
