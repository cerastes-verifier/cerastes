// expect: error check @ L34
// mode: c23, c++20
// twin: tests/memory/struct_array_global_loop_call.c
// Pins: 0063 item 10 (audit round 3 S1): the same program, but the check relies on g.a[1], which bump changed: g is havocked by the loop's calls, so the check fails (and the call's permission row does not fire first)
// Pre-registered from audit round 3 S1 and record 0063 item 10 (Part C), before any run: the one error
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
  CERASTES_CHECK(g.a[1] == 2);  // <- here
  int v = h - 5;
  v += g.n - g.n;
  return v;
}
