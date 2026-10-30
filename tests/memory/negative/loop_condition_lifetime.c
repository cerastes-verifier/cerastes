// expect: error lifetime @ L19
// mode: c23, c++20
// twin: tests/memory/loop_condition_cell.c
// Pins: 0038 item 2: the condition compares a pointer whose object's block ended
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int *p;
  {
    int y = 1;
    p = &y;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (p == &x && i < n) {
    i += 1;
  }
  return i;
}
