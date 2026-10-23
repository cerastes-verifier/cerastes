// expect: error termination @ L15
// mode: c23, c++20
// twin: tests/scalar/call_fib.c
// Pins: 0021: with the measure n / 2 the call fib(n - 1) does not decrease (n = 3); the measure is checked at a recursive call inside a sum
#include <cerastes.h>

int fib(int n)
    CERASTES_REQUIRES(n <= 1073741825)
    CERASTES_DECREASES(n / 2)
    CERASTES_ENSURES(r, r == n)
{
  if (n < 2) {
    return n;
  }
  return fib(n - 1) + fib(n - 2) - n + 3;
}

int use_fib(int m)
    CERASTES_REQUIRES(m <= 1073741825)
    CERASTES_ENSURES(r, r == m)
{
  return fib(m);
}
