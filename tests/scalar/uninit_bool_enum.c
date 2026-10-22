// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 1: bool and enumeration locals
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int pick(int a)
    CERASTES_ENSURES(r, (a > 0 && r == 1) || (a <= 0 && r == 2))
{
  bool b;
  enum color c;
  int n = 0;
  if (a > 0) {
    b = true;
    c = RED;
  } else {
    b = false;
    c = GREEN;
  }
  if (b) {
    n += 1;
  }
  if (c == GREEN) {
    n += 2;
  }
  return n;
}
