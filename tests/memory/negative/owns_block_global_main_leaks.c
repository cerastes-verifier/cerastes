// expect: error leak @ L27
// mode: c23, c++20
// twin: tests/memory/owns_block_global.c
// Pins: 0054 item 2 and 5 (question 123): a block reachable only from a NON-owning global is a leak at main's end (only owning globals are exempt, question 174)
// Pre-registered from record 0054 item 2 and 5: leak at main's closing brace
#include <cerastes.h>
#include <stdlib.h>

static int *g = NULL;

void install(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g) && cerastes_owns_block(g) && *g == 4)
{
  g = (int *)malloc(sizeof(int));
  if (g == NULL) {
    abort();
  }
  *g = 4;
}

int main(void)
{
  install();
  CERASTES_CHECK(*g == 4);
  return 0;
}  // <- here
