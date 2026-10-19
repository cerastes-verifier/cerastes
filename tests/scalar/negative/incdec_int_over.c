// expect: error overflow @ L10
// mode: c23, c++20
// twin: tests/scalar/incdec_int.c
// Pins: 0010 item 3: INT_MAX++ overflows
#include <cerastes.h>

int next(int i)
    CERASTES_ENSURES(r, r >= i)
{
  i++;
  return i;
}
