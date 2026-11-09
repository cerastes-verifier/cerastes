// expect: error leak @ L20
// mode: c23, c++20
// twin: tests/memory/owns_block_invariant_load.c
// Pins: 0054 item 4 (audit round 2, Part B, blocking hole, probe q2): a written site through a load (*pp == NULL || owns_block(*pp)) that the loop sets to NULL (*pp = NULL): the block *pp held is dropped; the loop does not allocate, so nothing replaces the dropped block: the iteration ends owning an allocation the loop does not carry: leak at the loop body's closing brace (the back-edge row)
// Pre-registered from audit round 2 (Part B): leak at the loop body's closing brace; the probe VERIFIED when this was written (the fix has landed and the line is confirmed)
#include <cerastes.h>
#include <stdlib.h>

void f(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
    CERASTES_ENSURES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(*pp == NULL || cerastes_owns_block(*pp))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    *pp = NULL;
    i += 1;
  }  // <- here
}
