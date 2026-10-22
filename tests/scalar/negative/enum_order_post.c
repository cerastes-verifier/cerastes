// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/enum_order.c
// Pins: 0019 item 10: the comparison is <=, so BLUE is warm and the contract says it is not
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int warm(enum color c)
    CERASTES_ENSURES(r, (c < BLUE && r == 1) || (c >= BLUE && r == 0))
{
  if (c <= BLUE) {
    return 1;
  }
  return 0;
}
