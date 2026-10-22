// expect: error shift @ L14
// mode: c23, c++20
// twin: tests/scalar/switch_case_dead.c
// Pins: 0017 item 5: a case value is checked on every input, although the switch is reached only when x > 1000
#include <cerastes.h>

int pick(int x)
    CERASTES_ENSURES(r, r == (x <= 1000 ? 0 : x == 1024 ? 1 : x == 2000 ? 2 : 3))
{
  if (x <= 1000) {
    return 0;
  }
  switch (x) {
  case 1 << 40:
    return 1;
  case 2 * 1000:
    return 2;
  default:
    return 3;
  }
}
