// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 item 7: ?: with a pointer arm and a null arm gives a pointer value (M4.5b1a cut 1 may move this to b1b)
#include <stddef.h>
#include <cerastes.h>

int conditional(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 1 : 0))
{
  int x = 0;
  int *p = c != 0 ? &x : NULL;
  if (p) {
    *p = 1;
  }
  return x;
}
