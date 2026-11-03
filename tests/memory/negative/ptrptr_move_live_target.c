// expect: error permission @ L13
// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/ptrptr_move.c
// Pins: 0045 item 7: the move target is given only by live(q): the callee cannot return owns(**pp) of an object it was not given (permission at the return) (question 112: more than one error at this line: the `**pp` and `*pp` rows; Silicon cannot show the blocks of the sibling places differ, so a callee taking part of one block's liveness lowers the other's)
#include <cerastes.h>

void move(int **pp, int *q)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && cerastes_live(q))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == q && cerastes_owns(**pp))
{
  *pp = q;
}  // <- here
