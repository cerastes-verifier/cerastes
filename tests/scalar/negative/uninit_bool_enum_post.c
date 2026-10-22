// expect: error postcondition @ L28
// mode: c23, c++20
// twin: tests/scalar/uninit_bool_enum.c
// Pins: 0019 item 1: bool and enumeration locals are assigned and read; the result is wrong
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
    c = BLUE;
  }
  if (b) {
    n += 1;
  }
  if (c == GREEN) {
    n += 2;
  }
  return n;
}
