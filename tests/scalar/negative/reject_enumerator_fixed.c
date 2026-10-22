// expect: rejected cerastes-enumerator_not_handled @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_switch_fixed.c
// Pins: 0019 item 11 rule 2: a fixed enumeration is held to the rule
#include <cerastes.h>

enum uc : unsigned char { U0, U1 };

int classify(enum uc e)
{
  switch (e) {
  case U0:
    return 1;
  }
  return 0;
}
