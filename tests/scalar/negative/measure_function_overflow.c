// expect: error overflow @ L9
// mode: c23, c++20
// twin: tests/scalar/measure_function_expression.c
// Pins: 0013 item 7: n * 100000 overflows for n up to 100000: overflow at the measure
#include <cerastes.h>

int down(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100000)
    CERASTES_DECREASES(n * 100000)
    CERASTES_ENSURES(r, r == 0)
{
  if (n == 0) {
    return 0;
  }
  return down(n - 1);
}
