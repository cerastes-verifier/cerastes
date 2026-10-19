// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/scalar/call_no_contract.c
// Pins: 0011 item 5: nothing says ident(a) == a
#include <cerastes.h>

int ident(int x)
{
  return x;
}

long use(int a)
    CERASTES_ENSURES(r, r == a)
{
  return ident(a);
}
