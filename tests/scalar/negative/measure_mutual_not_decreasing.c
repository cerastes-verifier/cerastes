// expect: error termination @ L32
// mode: c23, c++20
// twin: tests/scalar/measure_mutual.c
// Pins: 0013 item 2: is_odd calls is_even with the same n
// The cycle never ends for n > 0; the error is at the call that does not decrease.
#include <cerastes.h>

int is_odd(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0 || r == 1);

int is_even(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0 || r == 1)
{
  if (n == 0) {
    return 1;
  }
  return is_odd(n - 1);
}

int is_odd(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0 || r == 1)
{
  if (n == 0) {
    return 0;
  }
  return is_even(n);
}
