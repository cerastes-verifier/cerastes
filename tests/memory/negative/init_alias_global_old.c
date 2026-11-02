// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/init_alias_unmarked.c
// Pins: 0042 item 6 (audit S4): old(g) with p == &g reads the global's cell on entry, where its flag is not promised
#include <cerastes.h>

static int g = 0;

void f(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p) && p == &g)
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_old(g) == 0)  // <- here
{
  *p = 0;
}

int main(void)
{
  return g;
}
