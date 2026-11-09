// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/owns_block_global.c
// Pins: 0054 item 10: the same rule for a block site on a global: a partnerless ensures site resting on a global in a bodiless contract stays rejected (M6's contract manifest)
// Pre-registered from record 0054 item 2 ("Partnerless sites ... or a global") and item 10: rejected unsupported_annotation at the declaration
#include <cerastes.h>
#include <stdlib.h>

static int *g = NULL;

void install(void) CERASTES_REQUIRES(cerastes_owns(g)) CERASTES_ENSURES(cerastes_owns(g) && cerastes_owns_block(g));  // <- here

int main(void)
{
  install();
  free(g);
  g = NULL;
  return 0;
}
