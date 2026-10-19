// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 items 4 and 5: ++ on unsigned char wraps
#include <cerastes.h>

unsigned char wrap(unsigned char c)
    CERASTES_REQUIRES(c == 255)
    CERASTES_ENSURES(r, r == 0)
{
  c++;
  return c;
}
