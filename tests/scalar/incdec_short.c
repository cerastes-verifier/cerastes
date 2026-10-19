// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 5: ++ on a narrow type converts back, checked
#include <cerastes.h>

short next(short s)
    CERASTES_REQUIRES(s < 32767)
    CERASTES_ENSURES(r, r == s + 1)
{
  s++;
  return s;
}
