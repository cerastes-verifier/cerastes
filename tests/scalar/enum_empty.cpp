// expect: verified
// expect: native-ok
// Pins: 0018 items 1 and 3: an empty enumeration has the values 0 and 1 (the smallest bit-field that holds the empty list's range)
#include <cerastes.h>

enum Empty {};

Empty make(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1)
    CERASTES_ENSURES(r, static_cast<int>(r) == x)
{
  Empty e = static_cast<Empty>(x);
  return e;
}

int roundtrip(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1)
    CERASTES_ENSURES(r, r == x)
{
  return static_cast<int>(make(x));
}
