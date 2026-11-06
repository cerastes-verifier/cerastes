// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 (audit S11): the function borrows a block (owns_block in both lists) and parks it in an owning local: the entry value of the formal with an ensures block site is an owner, so the local's ending row holds
// Pre-registered from record 0052 item 3 (audit round 1, S12): verifies in both modes
#include <cerastes.h>

void park(int *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(cerastes_owns_block(p))
{
  CERASTES_OWNING int *v = p;
  *v = 1;
}
