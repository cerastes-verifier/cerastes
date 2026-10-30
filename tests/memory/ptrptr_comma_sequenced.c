// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2 (P62 A8): *pp = &y, **pp is sequenced by the comma: no compiler warns, cerastes accepts it
#include <cerastes.h>

int sequenced(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 0;
  int y = 5;
  int *p = &x;
  int **pp = &p;
  int r = 0;
  r = (*pp = &y, **pp);
  return r;
}
