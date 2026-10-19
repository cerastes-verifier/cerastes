// expect: error shift @ L11
// mode: c23, c++20
// twin: tests/scalar/shl_const.c
// Pins: 0010 item 8: left shift of a negative value is an error in both modes
// C++20 would define it; cerastes uses C23's stricter rule in both modes.
#include <cerastes.h>

int times16(int a)
    CERASTES_REQUIRES(a >= -100 && a <= 100)
{
  return a << 4;
}
