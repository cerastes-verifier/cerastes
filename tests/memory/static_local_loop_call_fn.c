// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 12 (audit round 2, SF1): the loop is in h, which reaches the static local only through f: h holds an implicit atom and the loop keeps it
#include <cerastes.h>

int f(void)
{
  static int c = 0;
  if (c < 100) {
    c++;
  }
  return c;
}

int h(void)
{
  int r = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    r = f();
    i++;
  }
  return r;
}

int main(void)
{
  int a = h();
  (void)a;
  return 0;
}
