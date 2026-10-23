// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 and 0020 item 5 (P37): a conditional expression statement whose arms are calls: each call runs only in its arm, and its precondition is checked there
#include <cerastes.h>

int g(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x && r > 0)
{
  return x;
}

int h(int y)
    CERASTES_REQUIRES(y > 1)
    CERASTES_ENSURES(r, r == y && r > 1)
{
  return y;
}

int two_calls(int c, int x, int y)
    CERASTES_REQUIRES(c ? x > 0 : y > 1)
    CERASTES_ENSURES(r, r == c)
{
  c ? g(x) : h(y);
  return c;
}

int call_or_zero(int c, int x)
    CERASTES_REQUIRES(c == 0 || x > 0)
    CERASTES_ENSURES(r, r == c)
{
  c ? g(x) : 0;
  return c;
}
