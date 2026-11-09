// expect: error leak @ L15
// mode: c23, c++20
// twin: tests/memory/owning_global_loop_init.c
// Pins: 0054 item 5 ("Stores") and audit round 2 (Parts C and D), probe h11: init() overwrites the block g may hold without freeing it
// Pre-registered from record 0054 item 5: leak at the assignment in init
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

void init(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  g = (int *)malloc(sizeof(int));  // <- here
  if (g != NULL) {
    *g = 1;
  }
}

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    init();
    i += 1;
  }
  return 0;
}
