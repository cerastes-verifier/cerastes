// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 12 (audit round 2, SF1): a loop in main whose body only calls f, which holds the static local count: main holds it from program_start, the call may change it, nothing else does
#include <cerastes.h>

static int g = 0;

int f(void)
{
  static int c = 0;
  if (c < 100) {
    c++;
  }
  return c;
}

int main(void)
{
  int r = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    r = f();
    i++;
  }
  CERASTES_CHECK(g == 0);
  r += g;
  return r;
}
