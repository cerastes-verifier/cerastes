// expect: error shift @ L12
// mode: c23, c++20
// twin: tests/scalar/switch_case_dead.c
// Pins: 0017 item 5: a case value is checked although the switch is never reached (if (0))
#include <cerastes.h>

int never(int x)
    CERASTES_ENSURES(r, r == 7)
{
  if (0) {
    switch (x) {
    case 1 << 40:
      return 1;
    default:
      return 2;
    }
  }
  return 7;
}
