// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// Pins: 0042 item 8: --strict-init: every local without an initializer is marked, a marked local's address and the caller's own marked parameter are passed to marked parameters, a static local is not asked for the marker
#include <cerastes.h>

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0)
{
  *p = 0;
}

void init_through(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0)
{
  init(p);
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_UNINIT int x;
  init_through(&x);
  return x;
}

int plain(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_ENSURES(r, r == n)
{
  int y = n;
  return y;
}

int tick(void)
    CERASTES_ENSURES(r, 1 <= r && r <= 100)
{
  static int s;
  if (s < 0 || s > 99) {
    s = 0;
  }
  s += 1;
  return s;
}
