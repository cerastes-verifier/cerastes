// expect: error postcondition @ L26
// mode: c23, c++20
// twin: tests/memory/struct_loop_pointer.c
// Pins: 0043 item 11: the loop leaves p->y at 2 * n, the ensures says n
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void fill(struct S *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->x == n && p->y == n)
{
  int i = 0;
  p->x = 0;
  p->y = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->x == i && p->y == 2 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->x += 1;
    p->y += 2;
    i += 1;
  }
}  // <- here
