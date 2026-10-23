// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 item 8: what clang's -Wself-assign does not warn about is accepted: an explicit cast, another operator on the right, a compound assignment, the outer assignment of x = y = x, and anything involving a macro
#include <cerastes.h>

#define SET(a, b) a = b

int cast(int x)
    CERASTES_ENSURES(r, r == x)
{
  x = (int)x;
  return x;
}

int plus_zero(int x)
    CERASTES_ENSURES(r, r == x)
{
  x = x + 0;
  return x;
}

int doubled(int x)
    CERASTES_REQUIRES(-1073741824 <= x && x <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * x)
{
  x += x;
  return x;
}

int outer(int x, int y)
    CERASTES_ENSURES(r, r == 0)
{
  x = y = x;
  return x - y;
}

int through_macro(int x)
    CERASTES_ENSURES(r, r == x)
{
  SET(x, x);
  return x;
}

#define X x

int macro_left(int x)
    CERASTES_ENSURES(r, r == x)
{
  X = x;
  return x;
}

int unary_plus(int x)
    CERASTES_ENSURES(r, r == x)
{
  x = +x;
  return x;
}
