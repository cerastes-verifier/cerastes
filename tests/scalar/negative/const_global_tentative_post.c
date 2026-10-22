// expect: error postcondition @ L16
// c-only: in C `static const int Z;` is a tentative definition, zero-initialized (C23 6.9.2); C++ requires an initializer
// twin: tests/scalar/const_global_tentative.c
// Pins: 0017 item 1: the zero value of a tentative definition is used (the ensures expects 1 more)
// use() is changed to the sum the wrong ensures would give, so the error is only the one in zero_plus.
#include <cerastes.h>

static const int Z;

int zero_plus(int x)
    CERASTES_REQUIRES(0 <= x && x < 1000)
    CERASTES_ENSURES(r, r == x + 1)
{
  CERASTES_CHECK(0 <= x);
  CERASTES_CHECK(x < 1000);
  return x + Z;
}

int use(void)
    CERASTES_ENSURES(r, r == 1001)
{
  int a = zero_plus(0);
  int b = zero_plus(999);
  return a + b;
}
