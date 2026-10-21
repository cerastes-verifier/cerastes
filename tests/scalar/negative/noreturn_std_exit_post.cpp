// expect: error postcondition @ L18
// twin: tests/scalar/noreturn_std_exit.cpp
// Pins: 0015 item 5: std::abort is not called on x <= 3 and std::exit not on x <= 5
// The ensures claims x <= 2, which fails for x == 3.
#include <cstdlib>
#include <cerastes.h>

int pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && x <= 2)
{
  if (x > 5) {
    std::exit(1);
  }
  if (x > 3) {
    std::abort();
  }
  return x;
}
