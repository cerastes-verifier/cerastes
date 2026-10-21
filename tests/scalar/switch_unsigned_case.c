// expect: verified
// expect: native-ok
// c-only: C converts a case value to the promoted type of the scrutinee; in C++ the narrowing -1 is ill-formed
// Pins: 0014 item 6: a case value is converted to the scrutinee's type (C): case -1 is UINT_MAX
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
