// expect: rejected cerastes-unsupported_declaration @ L17
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 8 (staging): an array declared inside a loop body is rejected until the loop rules of Part C land (this pin is deleted by Part C)
// Pre-registered from the coordinator's decision of report 3 of M4.5e1a (the code rejects it as unsupported_declaration)
#include <cerastes.h>

int sum(int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_ENSURES(r, r >= 0)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s >= 0)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int a[2] = {1, 2};
    s = s + a[0];
    i = i + 1;
  }
  return s;
}
