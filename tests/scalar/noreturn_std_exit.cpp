// expect: verified
// expect: native-ok
// Pins: 0015 item 5: std::exit and std::abort are the same routines as ::exit and ::abort (C linkage in both modes)
// Each branch ends the program, so only 0 <= x <= 5 reaches the return.
#include <cstdlib>
#include <cerastes.h>

int pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && x <= 3)
{
  if (x > 5) {
    std::exit(1);
  }
  if (x > 3) {
    std::abort();
  }
  return x;
}
