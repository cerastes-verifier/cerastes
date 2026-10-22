// expect: error conversion @ L8
// mode: c23, c++20
// twin: tests/scalar/enum_init.c
// Pins: 0018 item 10: the initializer (signed char)300 does not fit signed char: a conversion error at the enumerator
#include <cerastes.h>

enum {
  S = (signed char)300,
  T = 1 << 3,
  U = T + 1
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
