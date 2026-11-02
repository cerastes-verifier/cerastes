// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 item 6: after a must_init function the flag is promised, so a post-state read through an alias (*q with q == p) is framed by the marked atom
#include <cerastes.h>

void init_via(int *p CERASTES_MUST_INIT, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && q == p)
    CERASTES_ENSURES(cerastes_owns(*p) && q == p && *q == 0)
{
  (void)q;
  *p = 0;
}

int use_via(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x;
  init_via(&x, &x);
  return x;
}
