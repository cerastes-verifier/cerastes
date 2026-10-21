// expect: error missing-return @ L17
// mode: c23, c++20
// twin: tests/scalar/switch_no_default.c
// Pins: 0014 item 6 and 0012 item 9: a switch with no default can fall off the end of a non-void function
// The final return is gone: k not in {0, 1} reaches the closing brace.
#include <cerastes.h>

int name_of(int k)
    CERASTES_ENSURES(r, (k == 0 && r == 10) || (k == 1 && r == 11))
{
  switch (k) {
  case 0:
    return 10;
  case 1:
    return 11;
  }
}
