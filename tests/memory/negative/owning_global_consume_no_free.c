// expect: error leak @ L25
// mode: c23, c++20
// twin: tests/memory/owning_global_consume.c
// Pins: 0054 item 5 ("Every other function returns the block through its ensures atom on g, or frees it"): only main is exempt; destroy takes the cell with its block and neither frees nor returns it
// Pre-registered from record 0054 item 5: leak at the closing brace of destroy (g may hold a block)
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
}  // <- here

int main(void)
{
  init();
  destroy();
  return 0;
}
