// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/guarded_not_taken.c
// Pins: 0020 item 3: without the guard the assignment runs for d <= 0 and the function returns 11 - d
#include <cerastes.h>

int guarded(int d)
    CERASTES_REQUIRES(-1000 <= d)
    CERASTES_ENSURES(r, r == (d > 0 ? (10 - d > 3 ? 11 - d : 10 - d) : -1))
{
  int x = -1;
  if ((x = 10 - d) > 3) {
    return x + 1;
  }
  return x;
}
