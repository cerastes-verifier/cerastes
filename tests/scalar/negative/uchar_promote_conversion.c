// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/uchar_promote.c
// Pins: 0010 item 6 and 0022: the int sum 256 converted back to unsigned char does not fit
#include <cerastes.h>

unsigned char wrap_uc(unsigned char a)
    CERASTES_REQUIRES(a == 255)
{
  return a + 1;
}
