// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 8: owns(*p) && owns(g) in one contract (both full, so p != &g inside the callee), called with a local's address
#include <cerastes.h>

static int g = 3;

void two(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(g) && *p == 1 && g == 2)
{
  *p = 1;
  g = 2;
}

int main(void)
{
  int x = 0;
  two(&x);
  CERASTES_CHECK(x == 1 && g == 2);
  return g - 2;
}
