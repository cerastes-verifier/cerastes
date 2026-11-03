// expect: error invariant-preserved @ L22
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_chain_invariant.c
// Pins: 0044 item 12 (P69 a_trap2): the loop repoints p->q to u and writes u.x, the invariant p->q->x == t.x leaves the pointer free: invariant-preserved (the probe that would catch an unsound accept)
#include <cerastes.h>

struct L {
  struct L *q;
  int x;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r >= 0)
{
  struct L t = {0, 0};
  struct L u = {0, 0};
  struct L s = {&t, 0};
  struct L *p = &s;
  struct L *pu = &u;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p == &s && pu == &u && p->q->x == t.x)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->q = &u;
    u.x = 3;
    i += 1;
  }
  return p == &s && pu == &u ? 0 : 1;
}
