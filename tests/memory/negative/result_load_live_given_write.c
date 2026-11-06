// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/result_load_live_given.c
// Pins: 0052 item 2 (audit 3 B2, question 157): the given live(*pp) never gives access: writing through the loaded pointer is permission
// Pre-registered from audit round 3 (build/m4.5d2b/audit3.md) S2 B2: permission at the write
#include <cerastes.h>
#include <stddef.h>

static int g = 0;

void put(int **pp, int *p)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_live(p))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_live(*pp))
{
  *pp = p;
}

int main(void)
{
  int *x = NULL;
  put(&x, &g);
  *x = 1;  // <- here
  return 0;
}
