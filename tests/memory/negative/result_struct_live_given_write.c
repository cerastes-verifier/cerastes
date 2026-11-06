// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/result_struct_live_given.c
// Pins: 0052 item 2 (audit 3 B2, question 157): the given live(r.q) never gives access: writing through the member is permission
// Pre-registered from audit round 3 (build/m4.5d2b/audit3.md) S2 B2: permission at the write
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

int main(void)
{
  struct P s = same(&g);
  *s.q = 1;  // <- here
  return 0;
}
