// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 2: what follows a call of a function that does not return, across a comma, never runs
#include <stdlib.h>
#include <cerastes.h>

int bump(int v)
    CERASTES_REQUIRES(-2147483647 <= v)
    CERASTES_ENSURES(r, r == v - 1)
{
  return v - 1;
}

int never(int d)
    CERASTES_ENSURES(r, false)
{
  return (abort(), d);
}

int edge(void)
    CERASTES_ENSURES(r, r == -2147483647 - 1)
{
  return bump(-2147483647);
}

int after(int d)
    CERASTES_REQUIRES(d == 2)
    CERASTES_ENSURES(r, r == 1)
{
  return (d = bump(d), 1 / d);
}
