// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 5: x is assigned on the case 0 path, which falls into case 1; the assignment before the switch covers the dispatch to case 1
#include <cerastes.h>

int carry(int code)
    CERASTES_ENSURES(r, r == (code == 0 ? 10 : (code == 1 ? 5 : 0)))
{
  int x;
  int total = 0;
  x = 5;
  switch (code) {
  case 0:
    x = 10;
    [[fallthrough]];
  case 1:
    total = x;
  }
  return total;
}
