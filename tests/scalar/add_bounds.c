// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 3: signed addition; the operator is the check
// The bounds keep a + b inside int; the contract's own a + b is checked too (item 11).
#include <cerastes.h>

int add(int a, int b)
    CERASTES_REQUIRES(a >= 0 && a <= 1000000000 && b >= 0 && b <= 1000000000)
    CERASTES_ENSURES(r, r == a + b)
{
  return a + b;
}
