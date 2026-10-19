// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/conv_return.c
// Pins: 0010 item 5: the error is at the return
#include <cerastes.h>

signed char fit(int x)
    CERASTES_REQUIRES(x >= -128 && x <= 200)
{
  return x;
}
