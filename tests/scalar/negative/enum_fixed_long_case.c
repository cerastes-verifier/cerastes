// expect: error conversion @ L14
// c-only: C++ refuses the program (error #2362: the case value does not fit the promoted type)
// twin: tests/scalar/enum_fixed_switch.c
// Pins: 0018 items 5 and 6(ii): in C the case constant -1 converts into the enumeration itself (unsigned long), and -1 is not one of its values
#include <cerastes.h>

enum EUL : unsigned long { L0, L1 };

int pick(enum EUL e)
{
  switch (e) {
  case L0:
    return 1;
  case -1:
    return 2;
  default:
    return 0;
  }
}
