// expect: error conversion @ L11
// c-only: C converts a case value to the promoted type of the scrutinee and checks it; C++ refuses the narrowing -1 (EDG #2362)
// twin: tests/scalar/switch_unsigned_case_ok.c
// Pins: 0022 item 1 and 0014 item 6: C's case -1 over an unsigned int converts -1 to the promoted type: a conversion error (exit 1, never 3), no longer UINT_MAX
#include <cerastes.h>

unsigned int is_all_ones(unsigned int u)
    CERASTES_ENSURES(r, (u == 4294967295u && r == 1) || (u != 4294967295u && r == 0))
{
  switch (u) {
  case -1:
    return 1;
  default:
    return 0;
  }
}
