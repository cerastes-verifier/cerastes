// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 8: decreases(**pp) on a recursive function: the measure reads the entry heap, the recursive call's measure is the callee's at the arguments in the current heap
#include <cerastes.h>
#include <stddef.h>

void countdown(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && **pp >= 0)
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && **pp == 0)
    CERASTES_DECREASES(**pp)
{
  if (**pp > 0) {
    **pp -= 1;
    countdown(pp);
  }
}
