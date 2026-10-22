// expect: error conversion @ L7
// twin: tests/scalar/const_global_init_named.cpp
// Pins: 0017 item 4: an initializer that names another constant is checked as written (G + 300 does not fit a signed char)
#include <cerastes.h>

const int G = 1 << 3;
const int H = (signed char)(G + 300);
const int K = H * G - 1;
constexpr int C = H + 1;

int combine(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == x + 108)
{
  CERASTES_CHECK(0 <= x);
  CERASTES_CHECK(x <= 100);
  return x + G + H + K + C;
}

int use()
    CERASTES_ENSURES(r, r == 316)
{
  int a = combine(0);
  int b = combine(100);
  return a + b;
}
