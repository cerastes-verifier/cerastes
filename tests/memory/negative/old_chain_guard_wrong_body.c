// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/memory/old_chain_guard.c
// Pins: 0045 item 3 (audit B2): the body writes 1 instead of adding 1: postcondition (the post-state-disjunct form of old would let this verify, audit B2's wrong verdict)
#include <cerastes.h>
#include <stddef.h>

void f(int **pp, int *c)
    CERASTES_REQUIRES(cerastes_owns(*c) && cerastes_owns(*pp) && (*c == 0 || cerastes_owns(**pp)))
    CERASTES_REQUIRES(*c == 0 || **pp < 100)
    CERASTES_ENSURES(cerastes_owns(*c) && *c == 0 && cerastes_owns(*pp) && *pp == cerastes_old(*pp) && (cerastes_old(*c) == 0 || cerastes_owns(**pp)))
    CERASTES_ENSURES(cerastes_old(*c) == 0 || **pp == cerastes_old(**pp) + 1)
{
  if (*c != 0) {
    **pp = 1;
  }
  *c = 0;
}  // <- here
