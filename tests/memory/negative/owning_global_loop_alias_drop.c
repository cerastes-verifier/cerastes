// expect: error leak @ L25
// mode: c23, c++20
// twin: tests/memory/owning_global_loop_cycle.c
// Pins: 0054 item 5 ("Stores through an alias") and audit round 2 (Parts C and D), probe h8: the loop sets the owning global to NULL through an alias; a store through an alias gets no guarded row, so the dropped block is reported at the end label
// Pre-registered from record 0054 item 5 and audit probe h8 as observed: leak at main's closing brace
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

int main(void)
{
  g = (int *)malloc(sizeof(int));
  int **q = &g;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (i == 1) {
      *q = NULL;
    }
    i += 1;
  }
  return 0;
}  // <- here
