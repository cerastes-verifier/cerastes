// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2 (audit 3 B2, b3_load.c): a given live(*pp) on a place through a load of a formal: the caller passes the pointer it loaded on to a live-requiring function without an exit 3; the object is a global
// Pre-registered from audit round 3 (build/m4.5d2b/audit3.md) S2 B2: verifies in both modes once the fix lands (it exited 3)
#include <cerastes.h>
#include <stddef.h>

static int g = 0;

void put(int **pp, int *p)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_live(p))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_live(*pp))
{
  *pp = p;
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
  int *x = NULL;
  put(&x, &g);
  int k = peek(x);
  return k - 1;
}
