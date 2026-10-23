// expect: error division-by-zero @ L31
// mode: c23, c++20
// twin: tests/scalar/comma_noreturn.c
// Pins: 0020 item 2: for d == 1 the call leaves d == 0 and the division follows
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
    CERASTES_REQUIRES(d == 1 || d == 2)
    CERASTES_ENSURES(r, r == 1)
{
  return (d = bump(d), 1 / d);
}
