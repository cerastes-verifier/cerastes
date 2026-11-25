// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/global_array_calls.c
// Pins: 0063 item 8: helper's contract names no global, so it does not own g: the call of bump, which needs owns(g), fails permission at the call (a call's rows per array leaf)
// Pre-registered from record 0063 item 8 (Part B), before any run: the one error
#include <cerastes.h>

static int g[4] = {1, 2, 3, 4};

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[1] < 100)
    CERASTES_ENSURES(cerastes_owns(g))
{
  g[1] += 1;
}

void helper(void)
{
  bump();  // <- here
}
