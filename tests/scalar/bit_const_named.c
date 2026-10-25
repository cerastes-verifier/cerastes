// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 items 1 and 6 (0018): an enumerator initializer may name earlier enumerators, which are int in both modes
#include <cerastes.h>

enum { A = 1, B = 2, AB = A | B, AX = AB ^ A, NA = ~A };

int ab_value(void)
    CERASTES_ENSURES(r, r == 3)
{
  return AB;
}

int ax_value(void)
    CERASTES_ENSURES(r, r == 2)
{
  return AX;
}

int na_value(void)
    CERASTES_ENSURES(r, r == -2)
{
  return NA;
}
