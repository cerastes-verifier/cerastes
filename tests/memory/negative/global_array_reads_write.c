// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/global_array_calls.c
// Pins: 0063 item 7: reads(g) of an array global is a fraction of every element: a write g[1] needs the whole permission (global_reads_write.c's rule, per element)
// Pre-registered from record 0063 item 7 (Part B), before any run: the one error
#include <cerastes.h>

static int g[4] = {1, 2, 3, 4};

void poke(void)
    CERASTES_REQUIRES(cerastes_reads(g))
    CERASTES_ENSURES(cerastes_reads(g))
{
  g[1] = 5;  // <- here
}

int main(void)
{
  return g[0] - 1;
}
