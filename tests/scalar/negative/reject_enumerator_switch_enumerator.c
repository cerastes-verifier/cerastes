// expect: rejected cerastes-enumerator_not_handled @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11 (owner, 2026-10-22): a controlling expression that is an enumerator is an enumeration value in both modes (clang accepts, g++ refuses)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int classify(void)
{
  switch (RED) {
  case RED:
    return 1;
  case GREEN:
    return 2;
  }
  return 0;
}
