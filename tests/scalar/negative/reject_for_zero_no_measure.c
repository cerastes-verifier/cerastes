// expect: rejected cerastes-missing_decreases @ L11
// mode: c23, c++20
// twin: tests/scalar/measure_for.c
// Pins: 0013 item 3 and 0014 item 5: for (;0;) needs a decreases
#include <cerastes.h>

int test(int n)
    CERASTES_REQUIRES(0 <= n && n < 10)
{
  int i = 0;
  for (; 0;) {
    i += 1;
  }
  return i + n;
}
