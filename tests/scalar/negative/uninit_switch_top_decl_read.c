// expect: error uninitialized @ L19
// mode: c23, c++20
// twin: tests/scalar/uninit_case_past_decl.c
// Pins: 0019 item 6: the declaration at the top of the switch body is skipped by every case; the default reads it unassigned
#include <cerastes.h>

int top_decl(int code)
    CERASTES_REQUIRES(0 <= code && code <= 2)
    CERASTES_ENSURES(r, code != 0 || r == 3)
{
  int total = 0;
  switch (code) {
    int x;
  case 0:
    x = 3;
    total = x;
    break;
  default:
    total = x;
    break;
  }
  return total;
}
