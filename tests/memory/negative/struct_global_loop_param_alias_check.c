// expect: error check @ L31
// mode: c23, c++20
// twin: tests/memory/struct_global_loop_param_alias.c
// Pins: 0044 item 8 (re-audit 2): the same program claiming g.x == 2 after f: check at the call site's check
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
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && (i == 0 || s->x == 1))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    g.x = 1;
    i += 1;
  }
  if (i == 0) {
    s->x = 1;
  }
}
int main(void)
{
  f(&g);
  CERASTES_CHECK(g.x == 2);  // <- here
  return 0;
}
