// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/enum_order_fixed.c
// Pins: 0019 item 10: unsigned long base: the result is inverted
#include <cerastes.h>

enum big : unsigned long { B0, B1, B2 };

int not_last(enum big b)
    CERASTES_ENSURES(r, (b < B2 && r == 1) || (b >= B2 && r == 0))
{
  if (b < B2) {
    return 0;
  }
  return 0;
}
