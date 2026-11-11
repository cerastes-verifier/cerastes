// expect: rejected cerastes-unsupported_statement @ L17
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 8: a loop while an array is in an open scope is rejected until the loop rules of item 8 land (this pin is deleted by Part C) (lifted by Part C of M4.5e1a)
// Pre-registered from record 0059 item 8
#include <cerastes.h>

int count(int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_ENSURES(r, r == n)
{
  int a[3] = {1, 2, 3};
  int i = a[0] - 1;
  CERASTES_CHECK(a[0] == 1);
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i++;
  }
  return i;
}
