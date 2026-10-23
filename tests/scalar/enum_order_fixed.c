// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 10: ordering of a value of an enumeration with a fixed unsigned long base
#include <cerastes.h>

enum big : unsigned long { B0, B1, B2 };

int not_last(enum big b)
    CERASTES_ENSURES(r, r == (b < B2 ? 1 : 0))
{
  if (b < B2) {
    return 1;
  }
  return 0;
}

int ordered(enum big a, enum big b)
    CERASTES_ENSURES(r, r == (a <= b ? 1 : 0))
{
  if (a <= b) {
    return 1;
  }
  return 0;
}
