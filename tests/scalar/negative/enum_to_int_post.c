// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/enum_to_int.c
// Pins: 0018 item 3: 3 is a value of enum color, so the int can be 3, not at most 2
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int to_int(enum color c)
    CERASTES_ENSURES(r, r <= 2 && r == (int)c)
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
