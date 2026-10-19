// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/call_contract_only.c
// Pins: 0012 item 6: the body's r == 2x is not visible to the caller
#include <cerastes.h>

int twice(int x)
    CERASTES_REQUIRES(x >= 0 && x <= 1000)
    CERASTES_ENSURES(r, r >= x)
{
  return x * 2;
}

int use(int x)
    CERASTES_REQUIRES(x >= 0 && x <= 1000)
    CERASTES_ENSURES(r, r == x * 2)
{
  return twice(x);
}
