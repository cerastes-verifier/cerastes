// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/init_alias_unmarked.c
// Pins: 0042 item 6 (audit S4): a global g read through p == &g: the marked atom of p does not frame it
#include <cerastes.h>

static int g = 0;

void f(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p) && p == &g)
    CERASTES_REQUIRES(g == 0)  // <- here
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}

int main(void)
{
  return g;
}
