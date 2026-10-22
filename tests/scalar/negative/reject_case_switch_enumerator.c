// expect: rejected cerastes-case_not_in_enumeration @ L14
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11 rule 1 (owner, 2026-10-22): the same controlling expression with a case that is not an enumerator value
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int classify(void)
{
  switch (RED) {
  case 0:
    return 1;
  case 4:
    return 2;
  default:
    return 3;
  }
}
