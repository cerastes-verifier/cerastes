// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/result_two_atoms.c
// Pins: 0052 item 2: the callee holds only reads(g) and returns owns(*r) and live(r): the value cell is not held at write, so permission at the return
// Pre-registered from record 0052 item 2: permission at the return statement
#include <cerastes.h>

static int g = 3;

int *both(void)
    CERASTES_REQUIRES(cerastes_reads(g))
    CERASTES_ENSURES(r, cerastes_owns(*r) && cerastes_live(r) && r == &g)
{
  return &g;  // <- here
}
