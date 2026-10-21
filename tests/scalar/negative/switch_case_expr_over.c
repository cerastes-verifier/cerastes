// expect: error overflow @ L13
// c-only: C++ rejects the out-of-range constant expression in the front end (error #61)
// twin: tests/scalar/switch_case_expr.c
// Pins: 0014 item 6: a case value 2147483647 + 1 is an overflow error at the case (C only: C++ refuses it in the front end)
#include <cerastes.h>

int pick(int x)
    CERASTES_ENSURES(r, (x == 8 && r == 1) || (x == 3 && r == 2) || (x == 5 && r == 3) || (x != 8 && x != 3 && x != 5 && r == 0))
{
  switch (x) {
  case 1 << 3:
    return 1;
  case 2147483647 + 1:
    return 2;
  case (signed char)5:
    return 3;
  default:
    return 0;
  }
}
