// expect: error uninitialized @ L18
// mode: c23, c++20
// twin: tests/scalar/uninit_switch_fallthrough.c
// Pins: 0019 item 5: without the assignment before the switch, the dispatch to case 1 reaches the read with x unassigned; only case 0 falls in with a value
#include <cerastes.h>

int carry(int code)
    CERASTES_REQUIRES(0 <= code && code <= 1)
    CERASTES_ENSURES(r, code == 1 || r == 10)
{
  int x;
  int total = 0;
  switch (code) {
  case 0:
    x = 10;
    [[fallthrough]];
  case 1:
    total = x;
    break;
  }
  return total;
}
