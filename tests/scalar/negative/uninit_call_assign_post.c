// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/scalar/uninit_call_assign.c
// Pins: 0019 item 2: the call's result is assigned, then read; the result is wrong
#include <cerastes.h>

int ident(int v)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int use_ident(int a)
    CERASTES_REQUIRES(-1000 <= a && a <= 1000)
    CERASTES_ENSURES(r, r == a)
{
  int x;
  x = ident(a);
  x += 1;
  return x;
}
