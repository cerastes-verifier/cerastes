// expect: rejected cerastes-enumerator_not_handled @ L12
// mode: c23, c++20
// twin: tests/scalar/enum_switch_assign.c
// Pins: 0019 item 11 and 0020: a switch on (c = e) of enumeration type is a switch on an enumeration value; BLUE has no case and there is no default
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int f(int n)
{
  enum color c = RED;
  switch (c = (enum color)n) {
  case RED:
    return 1;
  case GREEN:
    return 2;
  }
  return (int)c;
}
