// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0022 item 1: a case value that fits the scrutinee's type (UINT_MAX over an unsigned int) selects exactly that value
#include <cerastes.h>

unsigned int is_all_ones(unsigned int u)
    CERASTES_ENSURES(r, r == (u == 4294967295u))
{
  switch (u) {
  case 4294967295u:
    return 1;
  default:
    return 0;
  }
}
