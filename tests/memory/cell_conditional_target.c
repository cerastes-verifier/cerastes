// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 3 (audit S1): *(*q > 0 ? a : b) = (*s)++ with s the address of a cell other than q's: the target's address (which reads *q) and the value (which writes *s) name different cells, so q != s holds
#include <cerastes.h>

int conditional_target(void)
    CERASTES_ENSURES(r, r == 6)
{
  int x = 1;
  int y = 2;
  int z = 3;
  int w = 4;
  int *a = &x;
  int *b = &y;
  int *q = &z;
  int *s = &w;
  *(*q > 0 ? a : b) = (*s)++;
  return x + y;
}
