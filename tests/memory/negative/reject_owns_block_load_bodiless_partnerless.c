// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owns_block_load_given.c
// Pins: 0054 item 10 (0052 item 2a extended to places through a formal's load): a partnerless ensures block site through a load in a bodiless contract (the declaration is called) would give the caller a second token for a block it may already hold: rejected, naming M6's contract manifest (stays rejected in d2b2a)
// Pre-registered from record 0054 item 2 ("Partnerless sites") and item 10: rejected unsupported_annotation at the declaration
#include <cerastes.h>
#include <stdlib.h>

void make(int **pp) CERASTES_REQUIRES(cerastes_owns(*pp)) CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp));  // <- here

int use_make(void)
{
  int *p = NULL;
  make(&p);
  free(p);
  return 0;
}
