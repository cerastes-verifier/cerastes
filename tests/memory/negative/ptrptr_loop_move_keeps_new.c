// expect: error permission @ L22
// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/ptrptr_loop_move.c
// Pins: 0045 item 10: a callee in the loop keeps the object *pp designates after the move: the back-edge row of its place fails (permission at the body's closing brace) (question 112: more than one error at this line: the `*q` and `*pp` rows; Silicon cannot show the blocks of the sibling places differ, so a callee taking part of one block's liveness lowers the other's)
#include <cerastes.h>
#include <stddef.h>
void keeper(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q));
void loop_move(int **pp, int *q, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && cerastes_owns(*q) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && cerastes_owns(*q))
{
  int *prev = *pp;
  *pp = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && *pp == q)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    keeper(*pp);
    i += 1;
  }  // <- here
  *pp = prev;
}
