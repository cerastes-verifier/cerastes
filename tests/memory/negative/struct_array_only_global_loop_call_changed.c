// expect: error check @ L32
// mode: c23, c++20
// twin: tests/memory/struct_array_only_global_loop_call.c
// Pins: 0063 item 10 (audit round 3 S1): the array-only structure global: the check relies on g.a[0], which bump changed; the loop havocs g, so the check fails and the call's permission row does not fire first
// Pre-registered from audit round 3 S1 and record 0063 item 10 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct W {
  int a[4];
};

static struct W g = {{1, 2, 3, 4}};
static int h = 5;

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  g.a[0] = 0;
}

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    bump();
    i += 1;
  }
  CERASTES_CHECK(g.a[0] == 1);  // <- here
  int v = h - 5;
  v += g.a[0] - g.a[0];
  return v;
}
