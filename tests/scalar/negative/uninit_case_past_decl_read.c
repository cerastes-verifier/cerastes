// expect: error uninitialized @ L18
// mode: c23, c++20
// twin: tests/scalar/uninit_case_past_decl.c
// Pins: 0019 item 6: the case after the declaration reads y without assigning it
#include <cerastes.h>

int mid_decl(int code)
    CERASTES_REQUIRES(0 <= code && code <= 1)
    CERASTES_ENSURES(r, code == 1 || r == 2)
{
  int total = 0;
  switch (code) {
  case 0:
    total = 2;
    break;
    int y;
  case 1:
    total = y;
    break;
  }
  return total;
}
