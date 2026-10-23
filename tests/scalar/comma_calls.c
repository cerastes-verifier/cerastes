// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 2: calls sequenced by a comma run in source order, and each precondition is checked
#include <cerastes.h>

int inc(int v)
    CERASTES_REQUIRES(v < 2147483647)
    CERASTES_ENSURES(r, r == v + 1)
{
  return v + 1;
}

int second(int x, int y)
    CERASTES_REQUIRES(x < 2147483647 && y < 2147483647)
    CERASTES_ENSURES(r, r == y + 1)
{
  int z = (inc(x), inc(y));
  return z;
}

int both(int x, int y)
    CERASTES_REQUIRES(0 <= y && y < 2147483647 && x <= 2147483645 - y)
    CERASTES_ENSURES(r, r == x + y + 2)
{
  int a;
  int b;
  a = inc(x), b = inc(y);
  return a + b;
}
