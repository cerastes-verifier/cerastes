// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 3 with 0015: the other arm ends the program, so only the assigning path reaches the read
#include <stdlib.h>
#include <cerastes.h>

int non_negative(int a)
    CERASTES_ENSURES(r, a >= 0 && r == a)
{
  int x;
  if (a >= 0) {
    x = a;
  } else {
    abort();
  }
  return x;
}
