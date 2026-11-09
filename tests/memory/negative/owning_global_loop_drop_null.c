// expect: error leak @ L19
// mode: c23, c++20
// twin: tests/memory/owning_global_loop_cycle.c
// Pins: 0054 item 5 and audit round 2 (Parts C and D), probe h7: the loop sets the owning global to NULL without freeing the block it holds
// Pre-registered from record 0054 item 5 ("Stores") and audit probe h7: leak at the store (the probe, which never read g, also drew EDG #550: a read after the loop is added)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

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
    i += 1;
  }
  free(g);
  return 0;
}
