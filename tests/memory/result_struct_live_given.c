// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2 (audit 3 B2, b3_struct.c): a given live(r.q) on a struct result's leaf: the caller passes the pointer it got on to a live-requiring function without an exit 3; the object is a global so that nothing is freed and no scope ends (a given amount is an unknown fraction to the caller)
// Pre-registered from audit round 3 (build/m4.5d2b/audit3.md) S2 B2: verifies in both modes once the fix lands (it exited 3)
#include <cerastes.h>

struct P {
  int *q;
};

static int g = 0;

struct P same(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_live(r.q))
{
  struct P w = {p};
  return w;
}

int peek(int *q)
    CERASTES_REQUIRES(cerastes_live(q))
    CERASTES_ENSURES(r, cerastes_live(q) && r == 1)
{
  (void)q;
  return 1;
}

int main(void)
{
  struct P s = same(&g);
  int k = peek(s.q);
  return k - 1;
}
