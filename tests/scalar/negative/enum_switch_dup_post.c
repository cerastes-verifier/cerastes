// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11: A and B have the same value, so case A covers B
#include <cerastes.h>

enum dup { A = 0, B = 0, C = 2 };

int dup_cover(enum dup d)
    CERASTES_ENSURES(r, (d == A && r == 1) || (d == C && r == 2) || (d != A && d != C && r == 0))
{
  switch (d) {
  case A:
    return 1;
  case C:
    return 2;
  }
  return 4;
}
