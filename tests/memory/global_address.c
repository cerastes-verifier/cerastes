// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 items 4 and 6: &g != &h and p == &g in functions that hold no permission to g or h (unique blocks; the liveness check passes by the GB_ disjunct; &g is not one past its object)
#include <cerastes.h>

static int g = 3;
static int h = 7;

int is_g(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && (r == 0 || r == 1))
{
  return p == &g;
}

int distinct(void)
    CERASTES_ENSURES(r, r == 1)
{
  CERASTES_CHECK(&g != &h);
  return 1;
}

int main(void)
{
  int x = 0;
  int a = is_g(&x);
  CERASTES_CHECK(a == 0 || a == 1);
  (void)a;
  h += 1;
  int d = distinct();
  CERASTES_CHECK(h == 8 && d == 1);
  (void)d;
  return 0;
}
