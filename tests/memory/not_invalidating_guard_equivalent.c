// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2 ("taken implies returned on every path"): the two guards are spelt differently (c > 0 and c >= 1), so atom_guard_shape does not match them and the new f__wf row prior ==> (g_req ==> g_ens) decides: it holds, and the converse row of today holds too
// Pre-registered from record 0058 item 2: verifies in both modes (without the marker, the same contract verifies today: build/m4.5d2b2d/emul/g1_today.c)
#include <cerastes.h>

void maybe(int *p CERASTES_NOT_INVALIDATING, int c)
    CERASTES_REQUIRES(c > 0 ? cerastes_owns(*p) : 1)
    CERASTES_ENSURES(c >= 1 ? cerastes_owns(*p) : 1);

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 1;
  maybe(&x, 1);
  maybe(&x, 0);
  return 0;
}
