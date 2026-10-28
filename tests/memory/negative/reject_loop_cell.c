// expect: rejected cerastes-unsupported_statement @ L14
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 10: a loop that touches a cell is rejected (re-expected by b1c)
#include <cerastes.h>

int loop_cell(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int *p = &x;
  CERASTES_INVARIANT(0 <= n && n <= 100)
  CERASTES_DECREASES(n)
  while (n > 0) {
    x += 1;
    n -= 1;
  }
  return *p;
}
