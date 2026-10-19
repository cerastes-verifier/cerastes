// expect: rejected cerastes-missing_decreases @ L12
// mode: c23, c++20
// twin: tests/scalar/measure_for.c
// Pins: 0013 item 3: a for loop without a decreases is rejected
#include <cerastes.h>

int count(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100)
{
  int i = 0;
  int c = 0;
  for (i = 0; i < n; i++) {
    c += 1;
  }
  return c;
}
