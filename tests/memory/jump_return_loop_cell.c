// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 items 3 and 7: a return inside a loop while a cell of an enclosing block is live (the shape that 0032 item 10 rejected): the loop holds the cell, the return exits it
#include <cerastes.h>

int return_in_loop(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 1 || r == 0)
{
  int x = 0;
  int *p = &x;
  CERASTES_INVARIANT(0 <= n && n <= 100)
  CERASTES_DECREASES(n)
  while (n > 0) {
    if (n == 50) {
      return 1;
    }
    n -= 1;
  }
  return *p;
}
