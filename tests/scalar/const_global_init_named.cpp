// expect: verified
// expect: native-ok
// Pins: 0017 items 3 and 4: in C++ an initializer may name other constants; each is checked as written
// G is 8, H is 10, K is 79 and C is 11.
#include <cerastes.h>

const int G = 1 << 3;
const int H = G + 2;
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
