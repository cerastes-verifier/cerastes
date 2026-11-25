// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 10 (audit round 3 S1): main's loop calls bump, which owns the structure global g (an array member and a cell): the loop keeps g's liveness, so the call's permission row holds; h, which bump does not name, keeps its value (twin negative/struct_array_global_loop_call_changed.c)
// Pre-registered from audit round 3 S1 and record 0063 item 10 (Part C), before any run: verifies (today a false permission: calls `bump`, which needs to own `g`, without owning it)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

static struct S g = {{1, 2, 3, 4}, 5};
static int h = 5;

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  g.n = 0;
  g.a[1] = 9;
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
  CERASTES_CHECK(h == 5);
  int v = h - 5;
  v += g.n - g.n;
  return v;
}
