// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 3: ++ on int is a signed addition
#include <cerastes.h>

int next(int i)
    CERASTES_REQUIRES(i < 2147483647)
    CERASTES_ENSURES(r, r == i + 1)
{
  i++;
  return i;
}
