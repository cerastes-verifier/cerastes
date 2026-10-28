// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 item 2 and 0018: a pointer to an accepted enumeration
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int pick_color(void)
    CERASTES_ENSURES(r, r == 1)
{
  enum color c = RED;
  enum color *pc = &c;
  *pc = GREEN;
  return c == GREEN;
}
