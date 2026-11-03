// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_chain_invariant.c
// Pins: 0044 item 12 (P69): the invariant reads p->q->x, which this loop does not hold (owns(*p) holds p->q, not the object it points to): permission at the clause
#include <cerastes.h>

struct L {
  struct L *q;
  int x;
};

int f(struct L *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->q != 0 && 0 <= n && n <= 10)
    CERASTES_ENSURES(r, r == 0 && cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->q->x == p->q->x)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return 0;
}
