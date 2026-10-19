// expect: rejected cerastes-missing_decreases @ L11
// expect: rejected cerastes-missing_decreases @ L21
// mode: c23, c++20
// twin: tests/scalar/measure_mutual.c
// Pins: 0013 item 4: mutual recursion across two functions without measures is rejected
// Each function of the cycle is reported, at its definition.
#include <cerastes.h>

int is_odd(int n);

int is_even(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == 0 || r == 1)
{
  if (n == 0) {
    return 1;
  }
  return is_odd(n - 1);
}

int is_odd(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == 0 || r == 1)
{
  if (n == 0) {
    return 0;
  }
  return is_even(n - 1);
}
