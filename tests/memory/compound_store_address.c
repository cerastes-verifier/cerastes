// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 5: a compound store whose address has a side effect evaluates the address once: *(q = p) += 1 and *(one() ? &x : &y) += 1 (refused before b1c2 as unsequenced and as a call reachable twice)
#include <cerastes.h>

int one(void)
    CERASTES_ENSURES(r, r == 1)
{
  return 1;
}

int compound_address(void)
    CERASTES_ENSURES(r, r == 6)
{
  int x = 1;
  int y = 10;
  int *p = &x;
  int *q = &y;
  *(q = p) += 1;
  *(one() ? &x : &y) += 1;
  return x + *q;
}
