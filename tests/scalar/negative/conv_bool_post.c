// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/conv_bool.c
// Pins: 0010 item 5: 0 converts to false, so the result is not always 1
#include <cerastes.h>

int truth(int x)
    CERASTES_ENSURES(r, r == 1)
{
  bool b = x;
  return b;
}
