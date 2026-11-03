// expect: error permission @ L25
// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/global_pointer_loop.c
// Pins: 0045 item 10: a callee in the body keeps *gp: the back-edge row of the automatic place fails (permission) (question 112: more than one error at this line: the `*gp` and `gp` rows; Silicon cannot show the blocks of the sibling places differ, so a callee taking part of one block's liveness lowers the other's)
#include <cerastes.h>
#include <stddef.h>
void keeper(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q));
static int h = 0;
static int *gp = &h;

void bump_n(int n)
    CERASTES_REQUIRES(cerastes_owns(gp) && cerastes_owns(*gp) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(gp) && gp == cerastes_old(gp) && cerastes_owns(*gp))
{
  int *gp0 = gp;
  (void)(gp0 == gp);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && gp == gp0)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    keeper(gp);
    i += 1;
  }  // <- here
}
