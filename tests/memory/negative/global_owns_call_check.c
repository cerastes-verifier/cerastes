// expect: error check @ L19
// mode: c23, c++20
// twin: tests/memory/global_owns_call.c
// Pins: 0041 items 7, 8: bump's ensures say g == old(g) + 1, so g == 3 does not hold after the call from g == 3
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
  bump();
  CERASTES_CHECK(g == 3);
  return 0;
}
