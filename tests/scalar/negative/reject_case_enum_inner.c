// expect: rejected cerastes-case_not_in_enumeration @ L16
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11 rule 1: the inner switch over an enumeration is checked too
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int classify(int n, enum color c)
{
  switch (n) {
  case 0:
    switch (c) {
    case RED:
      return 1;
    case 4:
      return 2;
    default:
      return 3;
    }
  default:
    return 0;
  }
}
