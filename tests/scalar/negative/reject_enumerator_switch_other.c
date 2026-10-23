// expect: rejected cerastes-enumerator_not_handled @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11 rule 2 (owner, 2026-10-22): an enumerator of another enumeration as controlling expression
#include <cerastes.h>

enum other { OA, OB };

int classify(void)
{
  switch (OA) {
  case OA:
    return 1;
  }
  return 0;
}
