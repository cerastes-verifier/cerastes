// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 items 3, 4, 6-9: a static global holds its initial value in main (program_start), owns(g) and old(g) at a call, the callee's ensures come back; main writes g between the calls
#include <cerastes.h>

static int g = 3;

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g < 100)
    CERASTES_ENSURES(cerastes_owns(g) && g == cerastes_old(g) + 1)
{
  g += 1;
}

int main(void)
{
  CERASTES_CHECK(g == 3);
  bump();
  CERASTES_CHECK(g == 4);
  g = 10;
  bump();
  CERASTES_CHECK(g == 11);
  return 0;
}
