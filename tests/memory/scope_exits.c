// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 item 8: a fall-through exit and returns from nested blocks that hold cells
#include <cerastes.h>

int scopes(int a)
    CERASTES_REQUIRES(0 < a && a < 1000)
    CERASTES_ENSURES(r, r == a + 3)
{
  int x = a;
  int *p = &x;
  {
    int w = 1;
    int *pw = &w;
    *pw = 3;
    x += *pw;
  }
  if (a > 500) {
    int y = *p;
    int *q = &y;
    {
      int z = *q;
      int *s = &z;
      if (*s > 0) {
        return *s;
      }
    }
    return y;
  }
  return x;
}
