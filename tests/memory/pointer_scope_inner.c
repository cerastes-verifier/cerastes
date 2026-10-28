// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 items 6, 8: a pointer to an inner block's local is used (read, compared) inside the block
#include <cerastes.h>

int inner_scope(int a)
    CERASTES_REQUIRES(-1000 < a && a < 1000)
    CERASTES_ENSURES(r, r == a + 1)
{
  int x = a;
  int *p = &x;
  {
    int y = 1;
    p = &y;
    if (p == &y) {
      x += *p;
    }
  }
  return x;
}
