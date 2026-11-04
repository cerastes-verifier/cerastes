// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/vlibc/exit_redeclared_after.c
// Pins: 0046 item 6 (question 116): the identical redeclaration adds no trust of its own; vlibc's contract is what ends the path
// The exit call is removed: the path of x < 0 returns and r >= 0 fails.
#include <stdlib.h>
#include <cerastes.h>

void exit(int status) CERASTES_ENSURES(false);

int nonneg(int x)
    CERASTES_REQUIRES(-10 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && r >= 0)
{
  if (x < 0) {
  }
  return x;
}
