// expect: rejected cerastes-unsupported_expression @ L12
// mode: c23, c++20
// twin: tests/memory/array_loop_sum.c
// Pins: 0059 item 8: an array used as a pointer in a loop invariant or measure is rejected (q == a; lifted by M4.5e1b with arrays in contracts)
#include <cerastes.h>

int walk(void)
{
  int a[3] = {1, 2, 3};
  int *q = &a[0];
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && q == a)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i += 1;
  }
  return *q;
}
