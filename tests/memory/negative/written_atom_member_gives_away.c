// expect: error permission @ L27
// expect: error permission @ L27
// mode: c23, c++20
// twin: tests/memory/written_atom_member.c
// Pins: 0045 item 12: an iteration gives *p->q away: the back-edge row of the atom fails (permission at the closing brace) (question 112: more than one error at this line: the `p->q->q` and `p->q` rows; Silicon cannot show the blocks of the sibling places differ, so a callee taking part of one block's liveness lowers the other's)
#include <cerastes.h>

struct L {
  struct L *q;
  int x;
};

void dispose_l(struct L *r)
    CERASTES_REQUIRES(cerastes_owns(*r));

void walk(struct L *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p->q) && 0 <= n && n <= 100)
{
  struct L *q0 = p->q;
  (void)(q0 == p->q);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->q == q0 && cerastes_owns(*p->q))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    dispose_l(p->q);
    i += 1;
  }  // <- here
}
