// expect: error permission @ L32
// mode: c23, c++20
// twin: tests/memory/owning_global_consume.c
// Pins: 0054 item 5: destroy consumed owns(g), the cell included: main's later read of g needs a permission main no longer holds
// Pre-registered from record 0054 item 5 ("a consuming callee clears held_g", the cell is gone): permission at the read
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

void destroy(void)
    CERASTES_REQUIRES(cerastes_owns(g))
{
  free(g);
}

int main(void)
{
  init();
  destroy();
  return g == NULL;  // <- here
}
