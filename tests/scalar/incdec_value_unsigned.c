// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1 and 0022: x++ on an unsigned type has the old value, and c ends one larger (exact below 255)
#include <cerastes.h>

int wrap(unsigned char c)
    CERASTES_REQUIRES(c <= 254)
    CERASTES_ENSURES(r, r == 2 * c + 1)
{
  int a = c++;
  return a + c;
}

int use(void)
    CERASTES_ENSURES(r, r == 509)
{
  return wrap(254);
}
