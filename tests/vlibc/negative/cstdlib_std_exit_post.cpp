// expect: error postcondition @ L17
// twin: tests/vlibc/cstdlib_std_exit.cpp
// Pins: 0046 item 8: std::abort's contract is what ends the path of 5 < x <= 7
// The std::abort call is removed: x == 6 reaches the return, and x <= 5 fails.
#include <cstdlib>
#include <cerastes.h>

int pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && x <= 5)
{
  if (x > 7) {
    std::_Exit(1);
  }
  if (x > 5) {
  }
  return x;
}
