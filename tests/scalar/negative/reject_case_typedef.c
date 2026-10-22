// expect: rejected cerastes-case_not_in_enumeration @ L15
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11 rule 1: a typedef of the enumeration does not hide it
#include <cerastes.h>

enum color { RED, GREEN, BLUE };
typedef enum color color_t;

int classify(color_t c)
{
  switch (c) {
  case RED:
    return 1;
  case 4:
    return 2;
  default:
    return 0;
  }
}
