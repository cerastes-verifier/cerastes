// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0024 item 5: the one spelling of a byte value above 127 as a character literal: u8'x' has type unsigned char (C23) or char8_t (C++20), so its value is 255
#include <cerastes.h>

int u8_max(void)
    CERASTES_ENSURES(r, r == 255)
{
  return u8'\xff';
}

int u8_letter(void)
    CERASTES_ENSURES(r, r == 97)
{
  return u8'a';
}

unsigned char u8_to_uchar(void)
    CERASTES_ENSURES(r, r == 255)
{
  unsigned char c = u8'\xff';
  return c;
}
