// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/incdec_value_unsigned.c
// Pins: 0020 item 1 and 0022: c++ gives c + 1 (not c + 2) for c <= 254
#include <cerastes.h>
int wrap(unsigned char c)
    CERASTES_REQUIRES(c <= 254)
    CERASTES_ENSURES(r, r == 2 * c + 2)
{
  int a = c++;
  return a + c;
}
