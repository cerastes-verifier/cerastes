// expect: error postcondition @ L18
// twin: tests/scalar/const_global_init_named.cpp
// Pins: 0017 item 3: the exact ensures notices a wrong sum of the constants
// use() is changed to the sum the wrong ensures would give, so the error is only the one in combine.
#include <cerastes.h>

const int G = 1 << 3;
const int H = G + 2;
const int K = H * G - 1;
constexpr int C = H + 1;

int combine(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == x + 109)
{
  CERASTES_CHECK(0 <= x);
  CERASTES_CHECK(x <= 100);
  return x + G + H + K + C;
}

int use()
    CERASTES_ENSURES(r, r == 318)
{
  int a = combine(0);
  int b = combine(100);
  return a + b;
}
