// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/pointer_enum.c
// Pins: 0032 item 2: the cell holds BLUE, not GREEN
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int pick_color(void)
    CERASTES_ENSURES(r, r == 1)
{
  enum color c = RED;
  enum color *pc = &c;
  *pc = BLUE;
  return c == GREEN;
}
