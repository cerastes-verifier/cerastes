// expect: error termination @ L14
// mode: c23, c++20
// twin: tests/memory/ptrptr_decreases_inner.c
// Pins: 0045 item 8: the body forgets to decrease **pp: termination at the recursive call
#include <cerastes.h>
#include <stddef.h>

void countdown(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && **pp >= 0)
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && **pp == 0)
    CERASTES_DECREASES(**pp)
{
  if (**pp > 0) {
    countdown(pp);  // <- here
  }
}
