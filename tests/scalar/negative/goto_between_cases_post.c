// expect: error postcondition @ L27
// mode: c23, c++20
// twin: tests/scalar/goto_between_cases.c
// Pins: 0015 item 2: the goto path of case 0 reaches the shared statement with kind == 0
// The ensures claims r == 7 for code == 0 (the value of case 2); only the goto path breaks it.
#include <cerastes.h>

int route(int code)
    CERASTES_REQUIRES(0 <= code && code <= 3)
    CERASTES_ENSURES(r, (code == 0 && r == 7) || (code == 1 && r == 1) || (code == 2 && r == 7) || (code == 3 && r == 0))
{
  int kind = 0;
  switch (code) {
  case 0:
    goto shared;
  case 1:
    kind = 1;
    break;
  case 2:
    kind = 2;
shared:
    kind += 5;
    break;
  default:
    break;
  }
  return kind;
}
