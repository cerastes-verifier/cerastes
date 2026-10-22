// expect: error shift @ L10
// c-only: C++ refuses the initializer 1 << 40 as not constant (error #28), so there is no enumerator to check
// twin: tests/scalar/enum_init.c
// Pins: 0018 item 10: the initializer 1 << 40 shifts past the width of int: a shift error at the enumerator
#include <cerastes.h>

enum {
  S = (signed char)100,
  T = 1 << 3,
  U = 1 << 40
};

int total(void)
    CERASTES_ENSURES(r, r == 117)
{
  return S + T + U;
}

int third(int x)
    CERASTES_REQUIRES(x <= 2147483638)
    CERASTES_ENSURES(r, r == x + 9)
{
  return x + U;
}

int third_edge(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return third(2147483638);
}
