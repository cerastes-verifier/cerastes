// expect: rejected cerastes-case_not_in_enumeration @ L14
// mode: c23, c++20
// twin: tests/scalar/enum_switch_fixed.c
// Pins: 0019 item 11 rule 1: a fixed unsigned char enumeration is held to the rule too
#include <cerastes.h>

enum uc : unsigned char { U0, U1 };

int classify(enum uc e)
{
  switch (e) {
  case U0:
    return 1;
  case 2:
    return 2;
  default:
    return 0;
  }
}
