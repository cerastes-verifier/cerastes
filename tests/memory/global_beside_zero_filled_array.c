// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 15 (M4.5e1e1 R4): a program with a global and a zero-filled array local (the trusted `zero_arr_int` carries no global-facts line, like `alloc_*`): the lint accepts it, and the zero rest is 0
// Pre-registered from record 0063 item 15 (R4), before the run: verifies
#include <cerastes.h>

static int g = 7;

int fill(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g < 100)
    CERASTES_ENSURES(r, cerastes_owns(g) && g == cerastes_old(g) + 1 && r == 1)
{
  int a[4] = {1};
  CERASTES_CHECK(a[0] == 1);
  CERASTES_CHECK(a[3] == 0);
  g += 1;
  return a[0] + a[3];
}

int main(void)
{
  int r = fill();
  CERASTES_CHECK(r == 1);
  CERASTES_CHECK(g == 8);
  return r - 1;
}
