// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 items 6, 7: a pointer that may be null is tested before it is dereferenced
#include <stddef.h>
#include <cerastes.h>

int guarded(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 7 : 0))
{
  int x = 0;
  int *p = NULL;
  if (c != 0) {
    p = &x;
  }
  if (p) {
    *p = 7;
  }
  return x;
}
