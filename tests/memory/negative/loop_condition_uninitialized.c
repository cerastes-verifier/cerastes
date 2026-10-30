// expect: error uninitialized @ L15
// mode: c23, c++20
// twin: tests/memory/loop_condition_cell.c
// Pins: 0038 item 2: the condition reads a cell that was never written
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n && x < n) {
    i += 1;
  }
  return i + (p == &x);
}
