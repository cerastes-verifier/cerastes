// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/result_live_given_same.c
// Pins: 0052 item 2 (audit B3, question 157): the given live(r) never gives access: writing through the pointer the callee gave back is permission at the write
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) B3: permission at the write
#include <cerastes.h>

static int g = 0;

int *same(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_live(r))
{
  return p;
}

int main(void)
{
  int *q = same(&g);
  *q = 1;  // <- here
  return 0;
}
