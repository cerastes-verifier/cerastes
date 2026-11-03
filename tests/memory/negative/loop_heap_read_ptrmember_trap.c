// expect: error invariant-preserved @ L21
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_ptrmember_invariant.c
// Pins: 0044 item 12 (P69 b_trap2): the loop repoints p->q to y and writes y, the invariant *p->q == x leaves the pointer free: invariant-preserved
#include <cerastes.h>

struct P {
  int *q;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r >= 0)
{
  int x = 0;
  int y = 0;
  struct P s = {&x};
  struct P *p = &s;
  int *py = &y;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p == &s && py == &y && *p->q == x)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->q = py;
    y = 3;
    i += 1;
  }
  return p == &s && py == &y ? x : 0;
}
