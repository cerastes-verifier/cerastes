// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/call_result_conversion.c
// Pins: 0012 item 6: the converted result keeps only what the callee promised, so the bound 99 is not provable
#include <cerastes.h>

int small(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r >= 0 && r <= 100)
{
  return x > 100 ? 100 : x;
}

signed char narrow_result(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r <= 99)
{
  return small(x);
}
