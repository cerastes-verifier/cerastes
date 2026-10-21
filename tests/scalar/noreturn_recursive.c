// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 4 and 0013: a recursive function that does not return has a measure like any other
// count_down recurses with n - 1 until abort(); the measure decreases at the recursive call.
#include <stdlib.h>
#include <cerastes.h>

[[noreturn]] void count_down(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
{
  if (n == 0) {
    abort();
  }
  count_down(n - 1);
}
