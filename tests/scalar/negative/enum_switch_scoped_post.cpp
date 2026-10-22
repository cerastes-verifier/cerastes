// expect: error postcondition @ L17
// mode: c++20
// twin: tests/scalar/enum_switch_scoped.cpp
// Pins: 0019 item 11 (C++): the named constant K is GREEN, so its case returns 2
#include <cerastes.h>

enum color { RED, GREEN, BLUE };
constexpr color K = GREEN;

int named(color c)
    CERASTES_ENSURES(r, (c == RED && r == 1) || (c == GREEN && r == 2) || (c == BLUE && r == 3) || (c == 3 && r == 0))
{
  switch (c) {
  case RED:
    return 1;
  case K:
    return 4;
  case BLUE:
    return 3;
  }
  return 0;
}
