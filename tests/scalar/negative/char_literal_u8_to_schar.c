// expect: error conversion @ L22
// mode: c23, c++20
// twin: tests/scalar/char_literal_u8.c
// Pins: 0024 item 5: char_literal_u8.c with a signed char destination: 255 does not fit, an ordinary conversion
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

signed char u8_to_uchar(void)
    CERASTES_ENSURES(r, r == 255)
{
  signed char c = u8'\xff';
  return c;
}
