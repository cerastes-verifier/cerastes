// expect: error permission @ L31
// mode: c23, c++20
// twin: tests/memory/struct_loop_roots.c
// Pins: 0043 items 9, 11: both owns atoms name the one object s: the separation fails at the call
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void both(struct S *p, struct S *q, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q) && p->x == n && q->y == n)
{
  int i = 0;
  p->x = 0;
  q->y = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->x == i && q->y == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->x += 1;
    q->y += 1;
    i += 1;
  }
}

int use_both(void)
{
  struct S s = {1, 2};
  both(&s, &s, 3);  // <- here
  return s.x;
}
