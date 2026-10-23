// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021: fib's shape, two unsequenced recursive calls with a measure (the result is linear, so the contract is exact); the bound is the largest n whose sum 2n - 3 fits
#include <cerastes.h>

int fib(int n)
    CERASTES_REQUIRES(n <= 1073741825)
    CERASTES_DECREASES(n)
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
