// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 4: a write of x before the store is sequenced by a comma or by the call of a function: x = (x++, 2) and x = g(x++)
#include <cerastes.h>

int g(int v)
    CERASTES_REQUIRES(v < 2147483647)
    CERASTES_ENSURES(r, r == v + 1)
{
  return v + 1;
}

int comma_store(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == 2)
{
  x = (x++, 2);
  return x;
}

int call_store(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  x = g(x++);
  return x;
}
