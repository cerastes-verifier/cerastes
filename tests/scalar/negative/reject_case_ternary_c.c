// expect: rejected cerastes-case_not_in_enumeration @ L14
// c-only: in C++ a ?: of enumeration type is rejected at the operator (0018 item 8), so the diagnostic differs
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11 rule 1 (owner, 2026-10-22): in C, ?: of two enumerators is an enumeration value, so the rules apply (C++ rejects a ?: of enumeration type, 0018 item 8)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int classify(int i)
{
  switch (i ? RED : GREEN) {
  case 0:
    return 1;
  case 4:
    return 2;
  default:
    return 3;
  }
}
