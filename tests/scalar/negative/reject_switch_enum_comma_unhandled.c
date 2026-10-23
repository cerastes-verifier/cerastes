// expect: rejected cerastes-enumerator_not_handled @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_switch_assign.c
// Pins: 0019 item 11 and 0020: a switch on (d++, c) of enumeration type is a switch on an enumeration value; BLUE has no case and there is no default
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int f(int d, enum color c)
{
  switch ((d++, c)) {
  case RED:
    return 1;
  case GREEN:
    return 2;
  }
  return d;
}
