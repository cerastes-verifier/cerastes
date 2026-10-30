// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 10 (audit round 2, SF1): a loop whose body only calls a function that owns g: g is an automatic place the call may change (its value is havocked, h, which the callee does not name, keeps its value)
#include <cerastes.h>

static int g = 1;
static int h = 5;

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  g = 9;
}

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    bump();
    i++;
  }
  CERASTES_CHECK(h == 5);
  int v = h - 5;
  v += g - g;
  return v;
}
