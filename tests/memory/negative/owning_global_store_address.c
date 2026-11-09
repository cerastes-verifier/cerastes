// expect: error permission @ L36
// mode: c23, c++20
// twin: tests/memory/owning_global_init_cleanup.c
// Pins: 0054 item 5 ("Stores", the value rule of 0052 item 3): an owning global receives only a pointer to a block the function holds whole, not the address of a local
// Pre-registered from record 0054 item 5 (S39 b1w): permission at the assignment
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

void init(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g);
  g = (int *)malloc(sizeof(int));
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

void point_at_local(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  int x = 0;
  free(g);
  g = &x;  // <- here
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
