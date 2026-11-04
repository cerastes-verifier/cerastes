// expect: error permission @ L20
// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/ptrptr_loop_inner.c
// Pins: 0045 item 10: a callee in the body keeps the object **pp: the back-edge row of the automatic place fails (permission at the body's closing brace) (question 112: more than one error at this line: the `**pp` and `*pp` rows; Silicon cannot show the blocks of the sibling places differ, so a callee taking part of one block's liveness lowers the other's)
#include <cerastes.h>
#include <stddef.h>
void keeper(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q));
void bump_n(int **pp, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    keeper(*pp);
    i += 1;
  }  // <- here
}
