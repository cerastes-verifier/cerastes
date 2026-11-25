// expect: error check @ L32
// mode: c23, c++20
// twin: tests/memory/struct_array_static_local_loop_call.c
// Pins: 0063 item 10 (audit round 3 S1): the same program with a wrong sum after the loop (3 calls of next, each returning 1): the check fails and the call's permission row does not fire first
// Pre-registered from audit round 3 S1 and record 0063 item 10 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int next(void)
    CERASTES_ENSURES(r, r == 1)
{
  static struct S c;
  c.a[1] = 1;
  return c.a[1];
}

int main(void)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && s == i)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s = s + next();
    i = i + 1;
  }
  CERASTES_CHECK(s == 2);  // <- here
  return s - 3;
}
