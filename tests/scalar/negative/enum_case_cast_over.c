// expect: error conversion @ L15
// mode: c23, c++20
// twin: tests/scalar/enum_case_cast.c
// Pins: 0018 items 5 and 6: a written cast in a case label is a checked conversion into the enumeration, 7 is not one of its values
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int pick(enum color c)
    CERASTES_ENSURES(r, (c == RED && r == 1) || (c != RED && r == 0))
{
  switch (c) {
  case RED:
    return 1;
  case (enum color)7:
    return 2;
  default:
    return 0;
  }
}
