// expect: error permission @ L27
// expect: error permission @ L27
// mode: c23, c++20
// twin: tests/memory/struct_chain_loop.c
// Pins: 0045 item 10: a callee in the body keeps p->q->x: the back-edge row of the automatic place fails (permission) (question 112: more than one error at this line: the `(p->q)->x` and `p->q` rows; Silicon cannot show the blocks of the sibling places differ, so a callee taking part of one block's liveness lowers the other's)
#include <cerastes.h>
void keeper(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q));

struct L {
  struct L *q;
  int x;
};

void bump_n(struct L *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(p->q->x) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(p->q->x))
{
  struct L *q0 = p->q;
  (void)(q0 == p->q);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->q == q0)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    keeper(&p->q->x);
    i += 1;
  }  // <- here
}
