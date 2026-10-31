// expect: error check @ L25
// mode: c23, c++20
// twin: tests/memory/global_loop_call.c
// Pins: 0041 item 10 (audit round 2, SF1): bump owns g and says nothing of its value, so after a loop that only calls it g == 1 does not hold (the loop may run the call)
#include <cerastes.h>

static int g = 1;

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
  CERASTES_CHECK(g == 1);
  return g - g;
}
