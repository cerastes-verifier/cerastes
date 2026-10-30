// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/global_owns_call.c
// Pins: 0041 item 8: helper's contract names no global, so it does not own g: the call of bump, which needs owns(g), fails permission at the call
#include <cerastes.h>

static int g = 3;

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g < 100)
    CERASTES_ENSURES(cerastes_owns(g) && g == cerastes_old(g) + 1)
{
  g += 1;
}

void helper(void)
{
  bump();
}
