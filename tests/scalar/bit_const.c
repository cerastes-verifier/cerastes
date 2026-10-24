// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 items 1 and 6: bitwise operators in named-constant initializers (checked in every prologue), enumerator initializers and case values
#include <cerastes.h>

const unsigned K = 240 | 15;
const int NM = ~7;
const unsigned char BYTE = 0xF0 | 0x0F;

enum { AB = 1 | 2, XR = 6 ^ 3, CM = ~1 };

unsigned k_value(void)
    CERASTES_ENSURES(r, r == 255u)
{
  return K;
}

int nm_value(void)
    CERASTES_ENSURES(r, r == -8)
{
  return NM;
}

int byte_value(void)
    CERASTES_ENSURES(r, r == 255)
{
  return BYTE;
}

int ab_value(void)
    CERASTES_ENSURES(r, r == 3)
{
  return AB;
}

int xr_value(void)
    CERASTES_ENSURES(r, r == 5)
{
  return XR;
}

int cm_value(void)
    CERASTES_ENSURES(r, r == -2)
{
  return CM;
}

int apply_mask(int x)
    CERASTES_ENSURES(r, r == (x & 255))
{
  return x & (int)K;
}

int apply_mask_value(void)
    CERASTES_ENSURES(r, r == 232)
{
  return apply_mask(1000);
}

int classify(int x)
    CERASTES_ENSURES(r, (x == 129 && r == 1) || (x == 5 && r == 2) || (x != 129 && x != 5 && r == 0))
{
  switch (x) {
  case 0x80 | 1:
    return 1;
  case 6 ^ 3:
    return 2;
  default:
    return 0;
  }
}
