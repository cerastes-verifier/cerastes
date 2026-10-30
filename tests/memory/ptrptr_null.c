// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2 (replaces reject_pointer_to_pointer.c, 0032 item 10): an int ** local compared with null
#include <cerastes.h>

int pointer_to_pointer(void)
    CERASTES_ENSURES(r, r == 1)
{
  int **pp = 0;
  return pp == 0;
}
