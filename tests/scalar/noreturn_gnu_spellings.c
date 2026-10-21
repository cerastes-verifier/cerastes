// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 1: [[gnu::noreturn]] and __attribute__((noreturn)) set the same flag as [[noreturn]]
// Both functions are declared here and defined elsewhere; a call of either ends the path.
#include <cerastes.h>

[[gnu::noreturn]] void stop_a(void);
__attribute__((noreturn)) void stop_b(void);

int pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && x <= 3)
{
  if (x > 6) {
    stop_a();
  }
  if (x > 3) {
    stop_b();
  }
  return x;
}
