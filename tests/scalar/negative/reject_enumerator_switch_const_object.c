// expect: rejected cerastes-enumerator_not_handled @ L12
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11 rule 2 (owner, 2026-10-22): a const object of the enumeration type as controlling expression (clang does not warn, GCC does)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int classify(void)
{
  const enum color k = RED;
  switch (k) {
  case RED:
    return 1;
  }
  return 0;
}
