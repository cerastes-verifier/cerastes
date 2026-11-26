// expect: error check @ L31
// mode: c23, c++20
// twin: tests/memory/global_array_struct_static_local.c
// Pins: 0064 item 9: the same program with a wrong sum after the loop (3 calls of next, each returning 1): the check fails and the call's permission row does not fire first
// Pre-registered from record 0064 item 9 (Part C2), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int next(void)
    CERASTES_ENSURES(r, r == 1)
{
  static struct P c[2];
  c[1].y = 1;
  return c[1].y;
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
