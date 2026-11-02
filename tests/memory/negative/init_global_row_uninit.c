// expect: error uninitialized @ L20
// mode: c23, c++20
// twin: tests/memory/init_global_row.c
// Pins: 0042 item 7 (audit S3): the call's flag row of a global's atom is `uninitialized`, naming the global: calls `h`, which needs `g` to have a value
#include <cerastes.h>

static int g = 0;

void h(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  g = 1;
}

void f(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p) && p == &g)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  h();  // <- here
}

int main(void)
{
  return g;
}
