// expect: error postcondition @ L24
// mode: c23, c++20
// twin: tests/scalar/comma_noreturn.c
// Pins: 0020 item 2: id() returns, so never() returns although it claims ensures false
#include <stdlib.h>
#include <cerastes.h>

int bump(int v)
    CERASTES_REQUIRES(-2147483647 <= v)
    CERASTES_ENSURES(r, r == v - 1)
{
  return v - 1;
}

int id(int v)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int never(int d)
    CERASTES_ENSURES(r, false)
{
  return (id(d), d);
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
