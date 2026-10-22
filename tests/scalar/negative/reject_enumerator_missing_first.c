// expect: rejected cerastes-enumerator_not_handled @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11 rule 2: the first enumerator without a case is named (RED)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int classify(enum color c)
{
  switch (c) {
  case GREEN:
    return 2;
  case BLUE:
    return 3;
  }
  return 0;
}
