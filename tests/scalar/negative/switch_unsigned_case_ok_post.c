// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/switch_unsigned_case_ok.c
// Pins: 0022 item 1: UINT_MAX matches the case, so r == 0 always is wrong
#include <cerastes.h>

unsigned int is_all_ones(unsigned int u)
    CERASTES_ENSURES(r, r == 0)
{
  switch (u) {
  case 4294967295u:
    return 1;
  default:
    return 0;
  }
}
