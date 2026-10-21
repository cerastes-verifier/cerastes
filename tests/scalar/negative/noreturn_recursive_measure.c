// expect: error termination @ L16
// mode: c23, c++20
// twin: tests/scalar/noreturn_recursive.c
// Pins: 0015 item 4 and 0013 item 2: the recursive call of a noreturn function must decrease the measure
// count_down(n) has the same measure as its caller: the termination error is at the call.
#include <stdlib.h>
#include <cerastes.h>

[[noreturn]] void count_down(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
{
  if (n == 0) {
    abort();
  }
  count_down(n);
}
