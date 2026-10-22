// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/scalar/enum_fixed_switch.c
// Pins: 0018 item 6(ii): the U1 case returns 21, which the ensures does not allow
#include <cerastes.h>

enum EU : unsigned { U0, U1, U2 };
enum EL : long { L0, L1, L2 };

int pick_unsigned(enum EU e)
    CERASTES_ENSURES(r, (e == U0 && r == 10) || (e == U1 && r == 20) || (e != U0 && e != U1 && r == 0))
{
  switch (e) {
  case U0:
    return 10;
  case U1:
    return 21;
  default:
    return 0;
  }
}

int pick_long(enum EL e)
    CERASTES_ENSURES(r, (e == L0 && r == 30) || (e == L1 && r == 40) || (e != L0 && e != L1 && r == 0))
{
  switch (e) {
  case L0:
    return 30;
  case L1:
    return 40;
  default:
    return 0;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 100)
{
  int a = pick_unsigned(U0);
  int b = pick_unsigned(U1);
  int c = pick_unsigned(U2);
  int d = pick_long(L0);
  int e = pick_long(L1);
  int f = pick_long(L2);
  return a + b + c + d + e + f;
}
