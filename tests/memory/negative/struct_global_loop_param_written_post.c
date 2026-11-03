// expect: error postcondition @ L30
// mode: c23, c++20
// twin: tests/memory/struct_global_loop_param_written.c
// Pins: 0044 item 8 (re-audit 2): the same program with s->x == 1 in the ensures (the invariant says nothing of the value written in the loop): postcondition at the closing brace
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S g = {0, 5};

void f(struct S *s)
    CERASTES_REQUIRES(cerastes_owns(*s) && s == &g)
    CERASTES_ENSURES(cerastes_owns(*s) && s->x == 1)
{
  int *p = &s->x;
  s = &g;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_owns(*p))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    g.x = 1;
    i += 1;
  }
  if (p == &g.x) {
    i = 0;
  }
}  // <- here

int main(void)
{
  f(&g);
  return 0;
}
