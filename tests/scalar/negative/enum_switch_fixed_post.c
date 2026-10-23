// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/scalar/enum_switch_fixed.c
// Pins: 0019 item 11: the unsigned long case returns 5, not 4
#include <cerastes.h>

enum uc : unsigned char { U0, U1 };
enum ul : unsigned long { L0, L1 };

int big(enum ul e)
    CERASTES_ENSURES(r, (e == L0 && r == 3) || (e == L1 && r == 4) || (e != L0 && e != L1 && r == 0))
{
  switch (e) {
  case L0:
    return 3;
  case L1:
    return 5;
  }
  return 0;
}
