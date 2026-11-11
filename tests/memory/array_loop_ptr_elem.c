// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 8: a loop over an array of int *: the pointer elements are kept (no write reaches the array), each one is stored through; the range conjunct for a pointer element is the WF of its value
// Pre-registered from record 0059 item 8 (Part C), before the code: verifies
#include <cerastes.h>

int count(void)
    CERASTES_ENSURES(r, r == 2)
{
  int x = 0;
  int y = 0;
  int *ps[2] = {&x, &y};
  int i = 0;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_INVARIANT(c == i)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    *ps[i] = 1;
    c = c + 1;
    i = i + 1;
  }
  return c;
}
