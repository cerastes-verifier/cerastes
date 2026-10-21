// expect: error shift @ L11
// mode: c23, c++20
// twin: tests/scalar/switch_case_expr.c
// Pins: 0014 item 6: a case value 1 << 40 is a shift error at the case (EDG folds it to 0)
#include <cerastes.h>

int pick(int x)
    CERASTES_ENSURES(r, (x == 8 && r == 1) || (x == 3 && r == 2) || (x == 5 && r == 3) || (x != 8 && x != 3 && x != 5 && r == 0))
{
  switch (x) {
  case 1 << 40:
    return 1;
  case 2 + 1:
    return 2;
  case (signed char)5:
    return 3;
  default:
    return 0;
  }
}
