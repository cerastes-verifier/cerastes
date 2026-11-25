// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 10 (audit round 3 S1): the array-only structure W (no cell leaf): main's loop calls bump, which owns the global g; the loop keeps g's liveness (the lone liveness row of an array-only structure global) (twin negative/struct_array_only_global_loop_call_changed.c)
// Pre-registered from audit round 3 S1 and record 0063 item 10 (Part C), before any run: verifies (today a false permission)
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
  CERASTES_CHECK(h == 5);
  int v = h - 5;
  v += g.a[0] - g.a[0];
  return v;
}
