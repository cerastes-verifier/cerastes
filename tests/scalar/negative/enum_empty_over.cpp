// expect: error conversion @ L12
// twin: tests/scalar/enum_empty.cpp
// Pins: 0018 item 3: an empty enumeration has the values 0 and 1, so 2 does not convert
#include <cerastes.h>

enum Empty {};

Empty make(int x)
    CERASTES_REQUIRES(0 <= x && x <= 2)
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
