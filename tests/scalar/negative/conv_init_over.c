// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/conv_init.c
// Pins: 0010 item 5: the error is at the initializer
#include <cerastes.h>

short narrow_init(int x)
    CERASTES_REQUIRES(x >= -32768 && x <= 32768)
{
  short s = x;
  return s;
}
