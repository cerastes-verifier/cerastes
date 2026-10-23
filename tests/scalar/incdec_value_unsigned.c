// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1: x++ on an unsigned type wraps; its value is the old value
#include <cerastes.h>

int wrap(unsigned char c)
    CERASTES_ENSURES(r, r == c + (c + 1) % 256)
{
  int a = c++;
  return a + c;
}
