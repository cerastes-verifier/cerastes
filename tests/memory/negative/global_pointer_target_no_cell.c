// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/global_pointer_target.c
// Pins: 0045 items 3, 4: owns(*gp) without owns(gp) or reads(gp) before it: the contract reads gp without owning it (permission at the clause)
#include <cerastes.h>
#include <stddef.h>

static int h = 7;
static int *gp = &h;

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(*gp))  // <- here
    CERASTES_ENSURES(cerastes_owns(*gp))
{
  *gp += 1;
}
