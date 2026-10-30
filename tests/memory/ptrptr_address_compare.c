// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2 (replaces reject_address_of_pointer.c, 0032 item 10): the address of a pointer local, copied to an int ** and compared with the address again
#include <cerastes.h>

int address_of_pointer(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  return pp == &p;
}
