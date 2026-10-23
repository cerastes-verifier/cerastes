// expect: rejected cerastes-case_not_in_enumeration @ L15
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11 rule 1: an enumerator of another enumeration whose value is not a value of this one
#include <cerastes.h>

enum color { RED, GREEN, BLUE };
enum other { OA = 7 };

int classify(enum color c)
{
  switch (c) {
  case RED:
    return 1;
  case OA:
    return 2;
  default:
    return 0;
  }
}
