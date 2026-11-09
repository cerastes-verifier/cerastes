// expect: error leak @ L16
// mode: c23, c++20
// twin: tests/memory/owning_global_two_declarations.c
// Pins: 0054 item 5: the same program whose reset overwrites the block g may hold without freeing it (the attribute on both declarations still makes g an owner, so the reassignment row applies)
// Pre-registered from record 0054 item 5 ("Stores"): leak at the assignment
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING extern int *g;
CERASTES_OWNING int *g = NULL;

void reset(void)
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
  g = NULL;
  reset();
  return 0;
}
