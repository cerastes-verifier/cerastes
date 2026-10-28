// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 item 6: (void)p and (void)(c ? p : &y) use the value of p; with p live they are fine
#include <cerastes.h>

int void_casts(int c)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 1;
  int y = 2;
  int *p = &x;
  (void)p;
  (void)(c != 0 ? p : &y);
  return y - 1;
}
