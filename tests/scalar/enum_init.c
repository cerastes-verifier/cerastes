// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0018 item 10: an enumerator initializer is encoded as written (a cast, a shift, another enumerator) and checked in every method's prologue
#include <cerastes.h>

enum {
  S = (signed char)100,
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
