// expect: verified
// expect: native-ok
// c-only: in C `static const int Z;` is a tentative definition, zero-initialized (C23 6.9.2); C++ requires an initializer
// Pins: 0017 item 1: a C static const definition without an initializer is a named constant with value 0
#include <cerastes.h>

static const int Z;

int zero_plus(int x)
    CERASTES_REQUIRES(0 <= x && x < 1000)
    CERASTES_ENSURES(r, r == x)
{
  CERASTES_CHECK(0 <= x);
  CERASTES_CHECK(x < 1000);
  return x + Z;
}

int use(void)
    CERASTES_ENSURES(r, r == 999)
{
  int a = zero_plus(0);
  int b = zero_plus(999);
  return a + b;
}
