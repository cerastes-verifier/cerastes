// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 9 (Part C2): a static local array of structures is a global with its implicit atom: next writes and reads one element, main's loop calls next, and the loop keeps the static's liveness and its atom (twin negative/global_array_struct_static_local_wrong.c)
// Pre-registered from record 0064 item 9 (Part C2), before any run: verifies
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
  CERASTES_CHECK(s == 3);
  return s - 3;
}
