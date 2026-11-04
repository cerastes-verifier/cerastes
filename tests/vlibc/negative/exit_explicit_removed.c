// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 8: the trusted `ensures(false)` of exit is what makes the positive verify; without the call x < 0 reaches the return
// The exit call is removed: the path of x < 0 returns, and r >= 0 fails.
#include <vlibc/stdlib.h>
#include <cerastes.h>

int nonneg(int x)
    CERASTES_REQUIRES(-10 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && r >= 0)
{
  if (x < 0) {
  }
  return x;
}
