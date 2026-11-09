// expect: error leak @ L18
// mode: c23, c++20
// twin: tests/memory/owning_global_loop_cycle.c
// Pins: 0054 item 5 and audit round 2 (Parts C and D), probe h2: the loop reallocates the owning global without freeing the block it holds (from the second iteration on)
// Pre-registered from record 0054 item 5 ("Stores") and audit probe h2 as observed: leak at the store
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {

    g = (int *)malloc(sizeof(int));  // <- here
    if (g != NULL) {
      *g = i;
    }
    i += 1;
  }
  return 0;
}
