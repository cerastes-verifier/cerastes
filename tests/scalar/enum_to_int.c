// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0018 item 6(iii): an enumeration value converts to int as a whole initializer; the value is one of 0 to 3
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int to_int(enum color c)
    CERASTES_ENSURES(r, r == (int)c)
{
  int i = c;
  return i;
}

int to_int_returned(enum color c)
    CERASTES_ENSURES(r, r == (int)c)
{
  return c;
}

int blue_index(void)
    CERASTES_ENSURES(r, r == 2)
{
  return to_int(BLUE);
}
