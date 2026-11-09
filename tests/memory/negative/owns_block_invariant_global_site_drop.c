// expect: error leak @ L29
// mode: c23, c++20
// twin: tests/memory/owns_block_invariant_global.c
// Pins: 0054 item 4 (audit round 2, Part B, blocking hole, probe q3): a written site on a non-owning global (g == NULL || owns_block(g)) that the loop sets to NULL; the loop does not allocate, so nothing replaces the dropped block: the iteration ends owning an allocation the loop does not carry: leak at the loop body's closing brace (the back-edge row)
// Pre-registered from audit round 2 (Part B): leak at the loop body's closing brace; the probe VERIFIED when this was written (the fix has landed and the line is confirmed)
#include <cerastes.h>
#include <stdlib.h>

static int *g = NULL;

int use_g(void)
    CERASTES_REQUIRES(cerastes_reads(g) && (g == NULL || cerastes_live(g)))
    CERASTES_ENSURES(r, cerastes_reads(g) && (g == NULL || cerastes_live(g)))
{
  return g == NULL;
}

void f(void)
    CERASTES_REQUIRES(cerastes_owns(g) && (g == NULL || cerastes_owns_block(g)))
    CERASTES_ENSURES(cerastes_owns(g) && (g == NULL || cerastes_owns_block(g)))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(g == NULL || cerastes_owns_block(g))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    g = NULL;
    i += 1;
  }  // <- here
}
