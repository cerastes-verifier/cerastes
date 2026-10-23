// expect: rejected cerastes-unsequenced_modification @ L10
// mode: c23, c++20
// twin: tests/scalar/incdec_value.c
// Pins: 0020 item 5: the modification of x is unsequenced with another use of x in the same full expression
#include <cerastes.h>

int f(int i)
    CERASTES_REQUIRES(0 <= i && i < 1000)
{
  return i++ + i;
}
