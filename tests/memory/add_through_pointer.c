// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 items 6, 9: *p += k at statement level reads and writes the cell, the operator is checked
#include <cerastes.h>

int add_through(int k)
    CERASTES_REQUIRES(0 <= k && k <= 1073741823)
    CERASTES_ENSURES(r, r >= k)
{
  int x = k;
  int *p = &x;
  *p += k;
  return x;
}
