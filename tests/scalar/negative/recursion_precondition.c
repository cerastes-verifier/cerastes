// expect: error precondition @ L15
// mode: c23, c++20
// twin: tests/scalar/recursion.c
// Pins: 0012 item 6: n - 2 is negative when n == 1, so the recursive call breaks the requires
#include <cerastes.h>

int count_down(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0)
{
  if (n == 0) {
    return 0;
  }
  return count_down(n - 2);
}
