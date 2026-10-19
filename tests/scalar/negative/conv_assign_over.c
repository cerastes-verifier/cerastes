// expect: error conversion @ L11
// mode: c23, c++20
// twin: tests/scalar/conv_assign.c
// Pins: 0010 item 5: the error is at the assignment
#include <cerastes.h>

short narrow_assign(int x)
    CERASTES_REQUIRES(x >= -32768 && x <= 32768)
{
  short s = 0;
  s = x;
  return s;
}
