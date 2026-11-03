// expect: error permission @ L28
// expect: error permission @ L28
// mode: c23, c++20
// twin: tests/memory/written_struct_leaf.c
// Pins: 0045 item 12: an iteration gives the owned leaf away: permission at the back edge (the body's closing brace) (question 112: more than one error at this line: the `s1.a` and `s2.a` rows; Silicon cannot show the blocks of the sibling places differ, so a callee taking part of one block's liveness lowers the other's)
#include <cerastes.h>
struct S {
  int a;
  int b;
};
void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int walk_s(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 1)
{
  struct S s1 = {1, 2};
  struct S s2 = {1, 3};
  struct S *q = &s1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s1.a == 1 && s2.a == 1 && (q == &s1 || q == &s2) && cerastes_owns(q->a) && q->a == 1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    dispose(&q->a);
    q = (q == &s1) ? &s2 : &s1;
    i += 1;
  }  // <- here
  return q->a;
}
