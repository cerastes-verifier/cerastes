// expect: error overflow @ L15
// mode: c23, c++20
// twin: tests/scalar/call_fib.c
// Pins: 0021: one more than the largest n: the sum of the two results overflows
#include <cerastes.h>

int fib(int n)
    CERASTES_REQUIRES(n <= 1073741826)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == n)
{
  if (n < 2) {
    return n;
  }
  return fib(n - 1) + fib(n - 2) - n + 3;
}

int use_fib(int m)
    CERASTES_REQUIRES(m <= 1073741826)
    CERASTES_ENSURES(r, r == m)
{
  return fib(m);
}
