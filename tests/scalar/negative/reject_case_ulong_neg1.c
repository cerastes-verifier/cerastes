// expect: rejected cerastes-case_not_in_enumeration @ L14
// c-only: C++ refuses the program (error #2362: the case value does not fit the promoted type)
// twin: tests/scalar/enum_switch_fixed.c
// Pins: 0019 item 11 rule 1: in C, case -1 over an unsigned long enumeration is not the value of an enumerator
#include <cerastes.h>

enum ul : unsigned long { L0, L1 };

int classify(enum ul e)
{
  switch (e) {
  case L0:
    return 1;
  case -1:
    return 2;
  default:
    return 0;
  }
}
