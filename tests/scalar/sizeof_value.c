// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 12: sizeof gives its value and is not a run-time operation
#include <cerastes.h>

int int_size(void)
    CERASTES_ENSURES(r, r == 4)
{
  return (int)sizeof(int);
}
