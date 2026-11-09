// expect: error leak @ L26
// mode: c23, c++20
// twin: tests/memory/owning_global_main_exit.c
// Pins: 0054 item 5: the same program with g NOT owning (the deep atom written out): the block it holds at main's end is a leak, only owning globals and owning static locals are exempt
// Pre-registered from record 0054 item 5 ("main"; S39 b1z): leak at main's closing brace
#include <cerastes.h>
#include <stdlib.h>

static int *g = NULL;

void fill(void)
    CERASTES_REQUIRES(cerastes_owns(g) && (g == NULL || cerastes_owns_block(g)))
    CERASTES_ENSURES(cerastes_owns(g) && (g == NULL || cerastes_owns_block(g)))
{
  free(g);
  g = (int *)malloc(sizeof(int));
  if (g != NULL) {
    *g = 1;
  }
}

int main(void)
{
  fill();
  return 0;
}  // <- here
