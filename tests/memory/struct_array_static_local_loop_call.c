// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 10 (audit round 3 S1): main's loop calls next, whose static structure local c (an array member and a cell) is an implicit atom the call needs: the loop keeps the static's liveness (twin negative/struct_array_static_local_loop_call_wrong.c)
// Pre-registered from audit round 3 S1 and record 0063 item 10 (Part C), before any run: verifies (today a false permission: calls `next`, which needs to own the `static` local `c` of `next`)
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
  CERASTES_CHECK(s == 3);
  return s - 3;
}
