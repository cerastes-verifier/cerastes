// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/switch_scrutinee.c
// Pins: 0014 item 6: x + 1 in the scrutinee may overflow
// Without the requires, x == INT_MAX overflows in the scrutinee.
#include <cerastes.h>

int is_minus_one(int x)
    CERASTES_ENSURES(r, (x == -1 && r == 1) || (x != -1 && r == 0))
{
  switch (x + 1) {
  case 0:
    return 1;
  default:
    return 0;
  }
}
