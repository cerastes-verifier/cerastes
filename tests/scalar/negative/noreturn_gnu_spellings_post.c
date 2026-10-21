// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/scalar/noreturn_gnu_spellings.c
// Pins: 0015 item 1: stop_b is not called for x <= 3, stop_a not for x <= 6
// The ensures claims x <= 2, which fails for x == 3.
#include <cerastes.h>

[[gnu::noreturn]] void stop_a(void);
__attribute__((noreturn)) void stop_b(void);

int pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && x <= 2)
{
  if (x > 6) {
    stop_a();
  }
  if (x > 3) {
    stop_b();
  }
  return x;
}
