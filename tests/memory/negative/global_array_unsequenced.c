// expect: error unsequenced @ L14
// mode: c23, c++20
// twin: tests/memory/global_array_sequencing.c
// Pins: 0063 item 11 (0039 item 3): without i != j the write and the increment may reach one element of the array global
// Pre-registered from record 0063 item 11 (Part B), before any run: the one error
#include <cerastes.h>

static int g[3] = {1, 2, 3};

int shift(int i, int j)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 3 && 0 <= j && j < 3 && 0 <= g[j] && g[j] < 100)
    CERASTES_ENSURES(r, cerastes_owns(g) && r == g[j])
{
  g[i] = g[j]++;  // <- here
  return g[j];
}

int main(void)
{
  return 0;
}
