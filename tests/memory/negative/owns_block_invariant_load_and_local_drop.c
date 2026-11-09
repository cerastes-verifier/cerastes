// expect: error leak @ L22
// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owns_block_invariant_load.c
// Pins: 0054 item 4 and owner question 186 (audit round 2, Part B, blocking hole; replaces the two-load-sites probe q4, 148 s): one written site through a load (*pp) and one local written site (p): the loop stores the local into *pp (*pp = p) and clears it, so the block *pp held is dropped; the loop does not allocate, so nothing replaces it and the iteration ends owning an allocation the loop does not carry: leak at the loop body's closing brace (the back-edge row). Pre-registered from audit round 2: the il-analyst's fix landed after the probes; the line is the closing brace of the loop body. Every reported line is registered (as owner questions 183 and 185): the back-edge leak at the loop body's closing brace and, as a consequence, the leak of the same block at the function's closing brace
#include <cerastes.h>
#include <stdlib.h>

void f(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
    CERASTES_ENSURES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
{
  int *p = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT((*pp == NULL || cerastes_owns_block(*pp)) && (p == NULL || cerastes_owns_block(p)))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    *pp = p;
    p = NULL;
    i += 1;
  }  // <- here
}
