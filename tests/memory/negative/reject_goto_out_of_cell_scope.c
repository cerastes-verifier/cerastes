// expect: rejected cerastes-unsupported_statement @ L14
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 10: a goto that leaves a block holding a cell is rejected (re-expected by b1c)
#include <cerastes.h>

int goto_cell(int a)
{
  int total = 0;
  {
    int x = a;
    int *p = &x;
    if (*p > 5) {
      goto done;
    }
    total = x;
  }
done:
  return total;
}
