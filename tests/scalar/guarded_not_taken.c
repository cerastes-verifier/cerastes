// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 3: when the guard is false the effect does not run: x keeps its value
#include <cerastes.h>

int guarded(int d)
    CERASTES_ENSURES(r, r == (d > 0 ? (10 - d > 3 ? 11 - d : 10 - d) : -1))
{
  int x = -1;
  if (d > 0 && (x = 10 - d) > 3) {
    return x + 1;
  }
  return x;
}
