// expect: verified
// expect: native-ok
// Pins: 0017 item 6: in C++ a case value may name a named constant, an expression over one, a local const or a local constexpr
// The values are 3, 4, 10 and 20, all distinct.
#include <cerastes.h>

const int N = 3;

int classify(int x)
    CERASTES_ENSURES(r, r == (x == 3 ? 1 : x == 4 ? 2 : x == 10 ? 3 : x == 20 ? 4 : 0))
{
  const int k = 10;
  constexpr int n = 20;
  switch (x) {
  case N:
    return 1;
  case N + 1:
    return 2;
  case k:
    return 3;
  case n:
    return 4;
  default:
    return 0;
  }
}
