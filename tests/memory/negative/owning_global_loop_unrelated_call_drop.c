// expect: error leak @ L26
// mode: c23, c++20
// twin: tests/memory/owning_global_loop_unrelated_call.c
// Pins: 0054 item 5 and audit round 2 (Parts C and D), probe a1: the same loop that also sets g to NULL on one iteration drops the block it holds
// Pre-registered from record 0054 item 5 ("Stores"): leak at the store
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

int sq(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x * x)
{
  return x * x;
}

int main(void)
{
  g = (int *)malloc(sizeof(int));
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (i == 1) {
      g = NULL;  // <- here
    }
    (void)sq(i);
    i += 1;
  }
  free(g);
  g = NULL;
  return 0;
}
