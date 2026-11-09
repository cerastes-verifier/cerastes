// expect: error leak @ L22
// mode: c23, c++20
// twin: tests/memory/owns_block_invariant_cond_place.c
// Pins: 0054 item 4 and audit round 2 (Parts C and D), probe c2: the loop clears *pp on the path where the guard holds the site, and nothing replaces the dropped block
// Pre-registered from record 0054 item 4 and audit probe c2: leak at the loop body's closing brace (the back-edge row; exit 3 "if.failed" beside it when written)
#include <cerastes.h>
#include <stdlib.h>

void f(int **pp, int c)
    CERASTES_REQUIRES(c == 0 || (cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp))))
    CERASTES_ENSURES(c == 0 || (cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp))))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(c == 0 || (*pp == NULL || cerastes_owns_block(*pp)))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (c != 0) {
      *pp = NULL;
    }
    i += 1;
  }  // <- here
}
