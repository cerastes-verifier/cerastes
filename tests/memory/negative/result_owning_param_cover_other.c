// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/result_owning_param_cover.c
// Pins: 0052 item 2: the same contract but the function returns the address of a global it does not hold: the owning parameter covers only where the result is p, so owns(*r) is given and not held: permission at the return
// Pre-registered from record 0052 item 2: permission at the return statement
#include <cerastes.h>
#include <stddef.h>

static int g = 3;

int *id(CERASTES_OWNING int *p)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns(*r))
{
  (void)p;
  return &g;  // <- here
}
