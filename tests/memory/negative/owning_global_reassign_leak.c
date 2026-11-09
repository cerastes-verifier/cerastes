// expect: error leak @ L15
// mode: c23, c++20
// twin: tests/memory/owning_global_init_cleanup.c
// Pins: 0054 item 5 ("Stores", 0052 item 3's reassignment row): init no longer frees the old block, and g may hold one: assigning to the owning global drops it
// Pre-registered from record 0054 item 5 (S39 b1x): leak at the assignment (the row ends the path: one error)
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

void cleanup(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g);
  g = NULL;
}

int main(void)
{
  init();
  init();
  cleanup();
  init();
  cleanup();
  return 0;
}
