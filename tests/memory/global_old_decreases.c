// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 7 (and 0039 item 8): old(g) in an ensures, owns(g) with a bound in a requires, decreases(g) over a global through a recursive call
#include <cerastes.h>

static int g = 5;

void inc(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g < 100)
    CERASTES_ENSURES(cerastes_owns(g) && g == cerastes_old(g) + 1)
{
  g += 1;
}

void count_down(void)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= g && g <= 100)
    CERASTES_ENSURES(cerastes_owns(g) && g == 0)
    CERASTES_DECREASES(g)
{
  if (g > 0) {
    g -= 1;
    count_down();
  }
}

int main(void)
{
  inc();
  CERASTES_CHECK(g == 6);
  count_down();
  CERASTES_CHECK(g == 0);
  return 0;
}
