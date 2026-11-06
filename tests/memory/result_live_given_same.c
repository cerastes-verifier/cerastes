// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2 (audit B3, i_live_given.c; the given amount is an unknown fraction to the caller): a callee keeps its live(p) and gives live(r) without saying r == p; the caller passes the pointer it got on to a live-requiring function and passes &g again: the call that used to exit 3 verifies (nothing is freed and no scope ends: a caller cannot prove it holds all of a block's liveness again after a given amount, record 0052 item 2)
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) B3: verifies in both modes (it exited 3 before the fix)
#include <cerastes.h>

static int g = 0;

int *same(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_live(r))
{
  return p;
}

int peek(int *q)
    CERASTES_REQUIRES(cerastes_live(q))
    CERASTES_ENSURES(r, cerastes_live(q) && (r == 0 || r == 1))
{
  return q != 0;
}

int main(void)
{
  int *q = same(&g);
  int k = peek(q);
  int j = peek(&g);
  return k + j;
}
