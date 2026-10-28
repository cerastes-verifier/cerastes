// expect: rejected cerastes-unsupported_expression @ L14
// mode: c23, c++20
// twin: tests/memory/cell_across_loop.c
// Pins: audit round 2 of M4.5b1a, 0032 item 10: a variable whose address is taken (a cell) in a loop measure is rejected (re-expected by b1c)
#include <cerastes.h>

int measure_cell(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i + x)
  while (i < n) {
    i += 1;
  }
  return *p;
}
