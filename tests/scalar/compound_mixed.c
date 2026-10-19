// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 6: i += u is computed in unsigned and converted back to int (checked)
// With i >= 0 and a small u the unsigned sum fits int.
#include <cerastes.h>

int add_unsigned(int i, unsigned u)
    CERASTES_REQUIRES(i >= 0 && i <= 1000 && u <= 1000u)
    CERASTES_ENSURES(r, r == i + (int)u)
{
  i += u;
  return i;
}
