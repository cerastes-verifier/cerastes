// expect: rejected cerastes-case_not_in_enumeration @ L14
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11 rule 1: case 4 is not the value of an enumerator of enum color
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int classify(enum color c)
{
  switch (c) {
  case RED:
    return 1;
  case 4:
    return 2;
  default:
    return 0;
  }
}
