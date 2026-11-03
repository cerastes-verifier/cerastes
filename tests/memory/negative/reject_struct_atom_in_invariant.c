// expect: rejected cerastes-unsupported_annotation @ L17
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 15, record 0044 item 1 (question 99): a written atom on a struct place in a loop invariant is rejected (0040 item 5's machinery is per cell; M4.5c2b)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void walk(struct S *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 10)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*p))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->x = i;
    i += 1;
  }
}
