// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 items 6, 7: a pointer arm of ?: is a use of the pointer; with p live the copy is fine
#include <cerastes.h>

int pick(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 0 : 1))
{
  int x = 1;
  int y = 2;
  int *p = &x;
  int *q = c != 0 ? p : &y;
  if (q == &y) {
    return 1;
  }
  return 0;
}
