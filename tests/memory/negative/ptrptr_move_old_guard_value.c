// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/memory/ptrptr_move_old_guard.c
// Pins: 0045 item 7: the same move, now claiming **pp keeps its value: after the move **pp is q's object, whose value need not be the old **pp: postcondition at the closing brace (the claim is checked on an object the callee owns at q's amount, so the twin fails for the value alone)
#include <cerastes.h>
#include <stddef.h>

void f(int **pp, int *q, int *c)
    CERASTES_REQUIRES(cerastes_owns(*c) && cerastes_owns(*pp) && cerastes_owns(**pp) && (*c != 0 || cerastes_owns(*q)))
    CERASTES_ENSURES(cerastes_owns(*c) && cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(**pp) && **pp == cerastes_old(**pp))
{
  if (*c == 0) {
    *pp = q;
    *c = 1;
  }
}  // <- here
