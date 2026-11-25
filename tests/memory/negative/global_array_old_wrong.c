// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/global_array_calls.c
// Pins: 0063 item 7: old(g[1]) is the element's entry value: g[1] == old(g[1]) + 2 fails after g[1] += 1 (postcondition at the closing brace of the void function)
// Pre-registered from record 0063 item 7 (Part B), before any run: the one error
#include <cerastes.h>

static int g[4] = {1, 2, 3, 4};

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[1] < 100)
    CERASTES_ENSURES(cerastes_owns(g) && g[1] == cerastes_old(g[1]) + 2)
{
  g[1] += 1;
}  // <- here

int main(void)
{
  return 0;
}
