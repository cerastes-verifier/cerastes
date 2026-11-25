// expect: error check @ L15
// mode: c23, c++20
// twin: tests/memory/global_beside_zero_filled_array.c
// Pins: 0063 item 15 (M4.5e1e1 R4): the twin of a global beside a zero-filled array local: a[3] is 0, not 1
// Pre-registered from record 0063 item 15 (R4), before the run
#include <cerastes.h>

static int g = 7;

int fill(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g < 100)
    CERASTES_ENSURES(r, cerastes_owns(g) && g == cerastes_old(g) + 1 && r == 1)
{
  int a[4] = {1};
  CERASTES_CHECK(a[3] == 1);
  g += 1;
  return a[0];
}

int main(void)
{
  int r = fill();
  CERASTES_CHECK(r == 1);
  CERASTES_CHECK(g == 8);
  return r - 1;
}
