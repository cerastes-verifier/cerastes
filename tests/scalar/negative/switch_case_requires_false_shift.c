// expect: error shift @ L11
// mode: c23, c++20
// twin: tests/scalar/switch_case_dead.c
// Pins: 0017 item 5: the case values are checked even when the requires is unsatisfiable
#include <cerastes.h>

int pick(int x)
    CERASTES_REQUIRES(x > 0 && x < 0)
{
  switch (x) {
  case 1 << 40:
    return 1;
  default:
    return 0;
  }
}
