// expect: error postcondition @ L22
// mode: c23, c++20
// twin: tests/scalar/uninit_case_past_decl.c
// Pins: 0019 item 6: the cases after a skipped declaration assign and read it; one returns the wrong value
#include <cerastes.h>

int mid_decl(int code)
    CERASTES_REQUIRES(0 <= code && code <= 1)
    CERASTES_ENSURES(r, (code == 0 && r == 2) || (code == 1 && r == 9))
{
  int total = 0;
  switch (code) {
  case 0:
    total = 2;
    break;
    int y;
  case 1:
    y = 8;
    total = y;
    break;
  }
  return total;
}
