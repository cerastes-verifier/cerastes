// expect: error conversion @ L17
// mode: c23, c++20
// twin: tests/scalar/call_result_conversion.c
// Pins: 0010 item 5: the callee only promises r <= 200, which does not fit signed char
#include <cerastes.h>

int small(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r >= 0 && r <= 200)
{
  return x > 100 ? 100 : x;
}

signed char narrow_result(int x)
    CERASTES_REQUIRES(x >= 0)
{
  return small(x);
}
