// expect: verified
// expect: native-ok
// c-only: _Noreturn is the C11 keyword; C++ has no such keyword (the attribute is [[noreturn]])
// Pins: 0015 item 1: the _Noreturn function specifier sets the same flag as [[noreturn]]
// _Noreturn leaves no attribute entry in the IL; the encoder reads the function type.
#include <stdlib.h>
#include <cerastes.h>

_Noreturn void bail(void)
{
  abort();
}

int checked_div(int a, int b)
    CERASTES_REQUIRES(-1000 <= a && a <= 1000)
    CERASTES_ENSURES(r, b != 0 && r == a / b)
{
  if (b == 0) {
    bail();
  }
  return a / b;
}
