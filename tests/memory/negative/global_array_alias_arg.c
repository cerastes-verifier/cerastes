// expect: error permission @ L21
// mode: c23, c++20
// twin: tests/memory/global_array_calls.c
// Pins: 0063 item 7 (the audit's alias twin): pair(&g[1]) against requires(owns(*p) && owns(g)): the cell *p lies in the array leaf, so the call's separation row fails (permission at the call); the positive passes another object
// Pre-registered from record 0063 item 7 (Part B), before any run: the one error
#include <cerastes.h>

static int g[4] = {1, 2, 3, 4};

int pair(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(g) && *p == 1 && g[0] == 2 && r == 0)
{
  *p = 1;
  g[0] = 2;
  return 0;
}

int main(void)
{
  pair(&g[1]);  // <- here
  return 0;
}
