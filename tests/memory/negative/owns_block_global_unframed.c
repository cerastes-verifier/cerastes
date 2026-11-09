// expect: error permission @ L13
// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/owns_block_global.c
// Pins: 0054 item 2 (framing): owns_block(g) reads the global g, and nothing in the ensures frames that read (owns(g), reads(g) or the implicit atom first): permission at the clause. Pre-registered from record 0054 item 2 ("When it is read"): permission at the ensures clause; after the first run (question 183): also permission at the return row (L20), whose body cannot hold the unframed place either, as an unframed owns(**pp) does
#include <cerastes.h>
#include <stdlib.h>

static int *g = NULL;

void install(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns_block(g))  // <- here
{
  g = (int *)malloc(sizeof(int));
  if (g == NULL) {
    abort();
  }
  *g = 4;
}
