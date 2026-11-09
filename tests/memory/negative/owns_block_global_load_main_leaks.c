// expect: error leak @ L27
// mode: c23, c++20
// twin: tests/memory/owns_block_global_load.c
// Pins: 0054 item 2 and 5: the block put() gave through *gp is reachable only from non-owning globals: a leak at main's closing brace
// Pre-registered from record 0054 item 2 (given) and item 5 (non-owning globals are not exempt at main's end): leak at the closing brace
#include <cerastes.h>
#include <stdlib.h>

static int *cell = NULL;
static int **gp = &cell;

void put(void)
    CERASTES_REQUIRES(cerastes_owns(gp) && cerastes_owns(*gp))
    CERASTES_ENSURES(cerastes_owns(gp) && gp == cerastes_old(gp) && cerastes_owns(*gp) && cerastes_owns_block(*gp))
{
  *gp = (int *)malloc(sizeof(int));
  if (*gp == NULL) {
    abort();
  }
  **gp = 1;
}

int main(void)
{
  put();
  return 0;
}  // <- here
