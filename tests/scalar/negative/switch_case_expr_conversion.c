// expect: error conversion @ L15
// mode: c23, c++20
// twin: tests/scalar/switch_case_expr.c
// Pins: 0014 item 6: a case value (signed char)300 is a conversion error at the case
#include <cerastes.h>

int pick(int x)
    CERASTES_ENSURES(r, (x == 8 && r == 1) || (x == 3 && r == 2) || (x == 5 && r == 3) || (x != 8 && x != 3 && x != 5 && r == 0))
{
  switch (x) {
  case 1 << 3:
    return 1;
  case 2 + 1:
    return 2;
  case (signed char)300:
    return 3;
  default:
    return 0;
  }
}
