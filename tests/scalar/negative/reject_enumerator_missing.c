// expect: rejected cerastes-enumerator_not_handled @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11 rule 2: BLUE has no case and there is no default
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int classify(enum color c)
{
  switch (c) {
  case RED:
    return 1;
  case GREEN:
    return 2;
  }
  return 0;
}
