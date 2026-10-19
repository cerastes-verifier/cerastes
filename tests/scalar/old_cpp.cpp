// expect: verified
// expect: native-ok
// Pins: 0011 item 7: cerastes::old is the one call allowed in an annotation (C++); old(x) of a by-value x is x
#include <cerastes.h>

int bump(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
    CERASTES_ENSURES(r, r == cerastes::old(x) + 1)
{
  x = x + 1;
  return x;
}
