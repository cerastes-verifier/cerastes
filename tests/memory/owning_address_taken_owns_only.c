// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 6 (&v passed to a callee with owns(*pp) alone, which may change v): after the call held_v := held_v && the token is held, and v's ending rows check its new value (here v was NULL and stays NULL)
// Pre-registered from record 0054 item 6: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void clear(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  *pp = NULL;
}

int use_clear(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = NULL;
  clear(&v);
  return 0;
}
