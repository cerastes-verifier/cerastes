// expect: error permission @ L27
// mode: c23, c++20
// twin: tests/memory/owning_global_init_cleanup.c
// Pins: 0054 item 5 ("Returns"): cleanup frees the block and leaves g non-null: it returns while the owning global does not hold its block ("set it to NULL after freeing it")
// Pre-registered from record 0054 item 5 (S39 b1y): permission at the return (the closing brace of this void function)
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
}  // <- here

int main(void)
{
  init();
  init();
  cleanup();
  init();
  cleanup();
  return 0;
}
