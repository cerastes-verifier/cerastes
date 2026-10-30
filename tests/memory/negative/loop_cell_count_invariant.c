// expect: error invariant-preserved @ L14
// mode: c23, c++20
// twin: tests/memory/loop_cell_count.c
// Pins: 0038 items 2 and 3: x == 2 * i holds on entry but not after x += 1; i += 1
#include <cerastes.h>

int count_cell(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 2 * n)
{
  int x = 0;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && x == 2 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    x += 1;
    i += 1;
  }
  return *p;
}
